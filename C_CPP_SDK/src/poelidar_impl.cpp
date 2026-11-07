/*
 * Copyright 2023-2024 Shanghai CPJRobot Co., Ltd. All rights reserved.
 * http://www.cpjrobot.com
 * http://www.poelidar.com
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include "poelidar_impl.h"

#include <string.h>
#include <iostream>
#include <chrono>

namespace cpj { namespace poelidar {


POELidarDriver*  POELidarDriver::CreateDriver(int model) {
    return new POELidarDriverImpl(model);
}

void POELidarDriver::DisposeDriver(POELidarDriver *drv) {
    delete drv;
}

POELidarDriverImpl::POELidarDriverImpl(int model)
{
    model_ = model;

    if (model_ == poelidar_model::F1) {
        socket_ = new cpj::net::SocketClient(cpj::net::SocketType::TCP);
        socket_udp_ = new cpj::net::SocketClient(cpj::net::SocketType::UDP);
    }

    if (model_ == poelidar_model::P2) {
        socket_ = new cpj::net::SocketClient(cpj::net::SocketType::TCP);
    }

    if (model_ == poelidar_model::M1) {
        socket_ = new cpj::net::SocketClient(cpj::net::SocketType::TCP);
    }

    is_start_ = false;
    is_connect_ = false;
    cmd_id_ = 0;
    cmd_buff_len_ = 0;
}

POELidarDriverImpl::~POELidarDriverImpl()
{
    disconnect();

    if(socket_){
        delete socket_;
        socket_ = NULL;
    }

    if(socket_udp_){
        delete socket_udp_;
        socket_udp_ = NULL;
    }

}

bool POELidarDriverImpl::connect(const std::string ip, const uint16_t port, uint32_t flag)
{
    if(is_connect_){return true;}

    switch (model_) {
      case poelidar_model::F1  : {
          if(port != 2105){
              std::cout <<"[POELidarDriverImpl]F1 port must be 2105!" << std::endl;
              return false;
          }

          if(!socket_->connect(ip, 2105)) {
              std::cout <<"[POELidarDriverImpl]connect failed!" << std::endl;
              return false;
          }
      } break;

      case poelidar_model::P2  : {
          if(!socket_->connect(ip, port)) {
              std::cout <<"[POELidarDriverImpl]connect failed!" << std::endl;
              return false;
          }
      } break;

      case poelidar_model::M1  : {
          if(!socket_->connect(ip, port)) {
              std::cout <<"[POELidarDriverImpl]connect failed!" << std::endl;
              return false;
          }
    } break;

      default :
          break;
    }

    ip_ = ip;

    is_connect_ = true;
    return true;
}

bool POELidarDriverImpl::disconnect()
{
    if(!is_connect_){return true;}

    stop();

    if (socket_) { socket_->close(); }

    if (socket_udp_) { socket_udp_->close(); }

    is_connect_ = false;

    return true;
}

bool POELidarDriverImpl::start()
{
    if(is_start_){return true;}

    switch (model_) {
    case poelidar_model::F1  : {
        if(!getUdpPort(udp_port_)){
            std::cout <<"[POELidarDriverImpl]get udp port error!" << std::endl;
            return false;
        }
        std::cout <<"[POELidarDriverImpl]F1 udp port: " << udp_port_
                  <<", please make sure this port is available!" << std::endl;

        if(!socket_udp_->connect(ip_, (uint16_t)udp_port_)) {
            std::cout <<"[POELidarDriverImpl]socket_udp_ connect failed!" << std::endl;
            return false;
        }

        get_cmd_(cmd_buff_, cmd_buff_len_, CPJ_F1_CMD_TYPE_START, cmd_id_);
        cmd_id_++;
        int real_send_size = socket_->send(cmd_buff_, cmd_buff_len_);
        if (real_send_size == 0) {
            std::cout <<"[POELidarDriverImpl]start(): send Head_F1_Scan error"<<std::endl;
            return false;
        }
    }
    break;

    case poelidar_model::M1  : {
        //power on first
        int send_size = socket_->send(M1_CMD_TYPE_PW_ON, sizeof(M1_CMD_TYPE_PW_ON));
        if (send_size == 0) {
            std::cout <<"[POELidarDriverImpl]start(): send M1_CMD_TYPE_PW_ON error"<<std::endl;
            return false;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));

        send_size = socket_->send(M1_CMD_TYPE_START, sizeof(M1_CMD_TYPE_START));
        if (send_size == 0) {
            std::cout <<"[POELidarDriverImpl]start(): send Head_M1_SCAN error"<<std::endl;
            return false;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    }
    break;

    default :
        break;

    }

    data_thread_enbale_ = true;//enbale data thread

    switch (model_) {
      case poelidar_model::F1  : {
          data_thread_ = std::thread(&POELidarDriverImpl::data_thread_F1_, this);
      } break;

      case poelidar_model::M1  : {
          data_thread_ = std::thread(&POELidarDriverImpl::data_thread_M1_, this);
      } break;

      default :
          break;
    }

    data_thread_.detach();

    is_start_ = true;

    return true;
}

bool POELidarDriverImpl::stop()
{
    if(!is_start_){return true;}

    data_thread_enbale_ = false;
    std::mutex m;
    std::unique_lock<std::mutex> lk(m);

    if (std::cv_status::timeout == cv_end_.wait_for(lk, std::chrono::milliseconds(1000))) {
        std::cout <<"[POELidarDriverImpl]stop data thread timeout!"<<std::endl;
        return false;
    }

    switch (model_) {
      case poelidar_model::F1  : {
          get_cmd_(cmd_buff_, cmd_buff_len_, CPJ_F1_CMD_TYPE_STOP, cmd_id_);
          cmd_id_++;
          int real_send_size = socket_->send(cmd_buff_, cmd_buff_len_);
          if (real_send_size == 0) {
              std::cout <<"[POELidarDriverImpl]start(): stop lidar error"<<std::endl;
              return false;
          }
      } break;

      case poelidar_model::M1  : {
          std::lock_guard<std::mutex> lk(mtx_);

          if(true) {
              int real_send_size = socket_->send(M1_CMD_TYPE_STOP, sizeof(M1_CMD_TYPE_STOP));
              if (real_send_size == 0) {
                  std::cout <<"[POELidarDriverImpl]stop(): send Head_M1_STOP error"<<std::endl;
                  return false;
              }
              std::cout <<"[POELidarDriverImpl]stop() OK!"<<std::endl;
          }
      } break;


      default :
          break;
    }

    is_start_ = false;

    return true;
}

bool POELidarDriverImpl::getScanData(poelidar_point_t *buffer, int &count, int timeout_ms)
{
    //TO DO
    std::cerr <<"[POELidarDriverImpl][getScanData]This feature is not available now, and we are still working on it!" << std::endl;
    return false;
}

bool POELidarDriverImpl::getScanData(std::vector<poelidar_point_t> &laser_scan,  int timeout_ms)
{
    {
    std::unique_lock<std::mutex> lk(mtx_);

    if(std::cv_status::timeout == cv_.wait_for(lk, std::chrono::milliseconds(timeout_ms))) {
        std::cout <<"[POELidarDriverImpl]getScanData timeout!"<<std::endl;
        return false;
    }

    if (cached_laser_scan_.empty()) {return false;}

    //std::cout <<"[POELidarDriverImpl::getScanData]Got valid laser_scan: "<<cached_laser_scan_.size()<<std::endl;

    laser_scan.clear();
    laser_scan = cached_laser_scan_;
    cached_laser_scan_.clear();

    }

    return true;
}

bool POELidarDriverImpl::getScanSpeed(int &speed_Hz)
{
    switch (model_) {
    case poelidar_model::F1  : {
        get_cmd_(cmd_buff_, cmd_buff_len_, CPJ_F1_CMD_TYPE_GET, cmd_id_, F1_PARAMS_SPEED, sizeof(F1_PARAMS_SPEED));
        cmd_id_++;

        int real_send_size = socket_->send(cmd_buff_, cmd_buff_len_);
        if (real_send_size == 0) {
            std::cout <<"[POELidarDriverImpl] send Head_F1_GET_SPEED error"<<std::endl;
            return false;
        }
        const int recv_buff_size = 2048;
        unsigned char recv_buff[2048];

        if(!socket_->waitfordata((uint32_t)1000)){
            std::cout <<"[POELidarDriverImpl]wait Head_F1_GET_SPEED data error"<<std::endl;
            return false;
        }

        int real_recv_size = socket_->recv(recv_buff, recv_buff_size);
        if (real_recv_size < 37) {
            std::cout <<"[POELidarDriverImpl]start(): recv Head_F1_GET_SPEED error,recv size"<<real_recv_size<<std::endl;
            return false;
        }
        char * result_addr = (char *)recv_buff + 26;
        char * result = strtok(result_addr, "," );
        float speed_f = (float)atof(result);

        if(8.0<speed_f && speed_f<12.0 ){
            speed_Hz = 10;
        }else if(13.0<speed_f && speed_f<17.0 ){
            speed_Hz = 15;
        }else if(18.0<speed_f && speed_f<22.0 ){
            speed_Hz = 20;
        }else if(23.0<speed_f && speed_f<27.0 ){
            speed_Hz = 25;
        }else if(28.0<speed_f && speed_f<32.0 ){
            speed_Hz = 30;
        }else{
            std::cout <<"[POELidarDriverImpl]speed_f:"<<speed_f<<std::endl;
            return false;
        }
    }  break;

    case poelidar_model::M1  : {
        speed_Hz = 10;
    }  break;

    default :
        break;

    }

    return true;
}

bool POELidarDriverImpl::getUdpPort(int &udp_port)
{
    switch (model_) {
    case poelidar_model::F1  : {
        get_cmd_(cmd_buff_, cmd_buff_len_, CPJ_F1_CMD_TYPE_GET, cmd_id_, F1_PARAMS_DATA_PORT, sizeof(F1_PARAMS_DATA_PORT));
        cmd_id_++;

        int real_send_size = socket_->send(cmd_buff_, cmd_buff_len_);
        if (real_send_size == 0) {
            std::cout <<"[POELidarDriverImpl] send Head_F1_Get_Udp_Port error"<<std::endl;
            return false;
        }
        const int recv_buff_size = 2048;
        unsigned char recv_buff[2048];

        if(!socket_->waitfordata((uint32_t)1000)){
            std::cout <<"[POELidarDriverImpl]wait Head_F1_Get_Udp_Port data error"<<std::endl;
            return false;
        }

        int real_recv_size = socket_->recv(recv_buff, recv_buff_size);
        if (real_recv_size < 37) {
            std::cout <<"[POELidarDriverImpl]start(): recv Head_F1_Get_Udp_Port error,recv size"<<real_recv_size<<std::endl;
            return false;
        }
        char * result_addr = (char *)recv_buff + 26;
        char * result = strtok(result_addr, "," );
        udp_port = (int)atof(result);
    }break;

    case poelidar_model::M1  : {
        udp_port = -1;
        std::cerr <<"[POELidarDriverImpl]Error, M1 has no UDP port!" << std::endl;
        return false;
    }  break;

    default :
        return false;
        break;

    }

    return true;
}

bool POELidarDriverImpl::setIP(std::string ip)
{
    //TO DO
    std::cerr <<"[POELidarDriverImpl][setIP]This feature is not available now, and we are still working on it!" << std::endl;
    return false;
}

bool POELidarDriverImpl::setScanSpeed(int speed_Hz)
{
    //TO DO
    std::cerr <<"[POELidarDriverImpl][setScanSpeed]This feature is not available now, and we are still working on it!" << std::endl;
    return false;
}

bool POELidarDriverImpl::setUdpPort(int udp_port)
{
    //TO DO
    std::cerr <<"[POELidarDriverImpl][setUdpPort]This feature is not available now, and we are still working on it!" << std::endl;
    return false;
}

void POELidarDriverImpl::data_thread_P2_(void)
{
    //TO DO
    std::cerr <<"[POELidarDriverImpl][setIP]This feature is not available now, and we are still working on it!" << std::endl;
    return;
}

void POELidarDriverImpl::data_thread_M1_(void)
{
    printf("data_thread_M1_ begin\n");

    std::vector<poelidar_point_t>  laser_scan;
    size_t    cnt = 0;
    const int kSkipFrameCnt = 35;//skip unstable data at the beginning


    DataPackM1 m1_pack;
    unsigned char* recvBuffPtr = (unsigned char*)&m1_pack;
    float pre_frame_last_pt_angle = 0;

    while (data_thread_enbale_)
    {
        //read sync head
        bool is_read_head_OK = false;
        int real_recv_size = 0;
        if(tcp_read_data_(recvBuffPtr, 1, real_recv_size, 1000)){
            if(*recvBuffPtr == M1_RES_SYNC_HEADER){
                if(tcp_read_data_(recvBuffPtr+1, 1, real_recv_size, 1000)){
                    uint8_t value = *(recvBuffPtr + 1);
                    if ( (value & 0x1F) == M1_NODE_NUM_PER_PACK) {
                        is_read_head_OK = true;
                    } else {
                        continue;
                    }
                }
            }else{
                continue;
            }
        }else{
            std::cout<<"[POELidarDriverImpl::data_thread_M1_]Error:real_recv_size < pack_size!"<< std::endl;
            return;
        }


        if(!is_read_head_OK) return;


        //read remain data
        const size_t res_size = sizeof(m1_pack) - 2;
        if(!tcp_read_data_(recvBuffPtr+2, res_size, real_recv_size, 1000)){
            std::cout<<"[POELidarDriverImpl::data_thread_M1_]Error:real_recv_size < pack_size!"<< std::endl;
            return;
        }

        //crc check
        bool is_crc_ok = false;
        uint8_t crc_result = m1_cal_crc8_((const uint8_t *)(&m1_pack), sizeof(m1_pack)-1 );
        if (crc_result == m1_pack.CRC8) { is_crc_ok = true; }

        if (!is_crc_ok) {
            std::cout<<"[POELidarDriverImpl::data_thread_M1_]Error:CRC check failed!"<< std::endl;
            continue;
        }

        //parse data
        float angle_increment;
        if(m1_pack.endAngle < m1_pack.startAngle) {
            angle_increment = (float)(m1_pack.endAngle + 36000 - m1_pack.startAngle) / (float)(M1_NODE_NUM_PER_PACK - 1);
        } else {
            angle_increment = (float)(m1_pack.endAngle - m1_pack.startAngle) / (float)(M1_NODE_NUM_PER_PACK - 1);
        }

        //printf("m1 start angle: %f, end angle:%f\n", m1_pack.startAngle*0.01f, m1_pack.endAngle*0.01f);
        //printf("m1 angle increment: %f\n", angle_increment*0.01f);
        //printf("m1 speed: %f\n", m1_pack.speed);

        float angle_pre = pre_frame_last_pt_angle;
        bool is_got_one_frame = false;
        int cross_frame_pt_idx = 0;
        for (int pos = 0; pos < M1_NODE_NUM_PER_PACK; pos++) {
            float angle_now = ((float)m1_pack.startAngle + angle_increment*pos) * 0.01f;
            angle_now = (angle_now >= 360.0f) ? (angle_now - 360.0f) : angle_now;
            if (angle_now < angle_pre) {

                cnt++;
                //std::cout<<"[POELidarDriverImpl::data_thread_M1_]cnt:"<< cnt<<std::endl;

                if(cnt > kSkipFrameCnt) {
                    mtx_.lock();
                    cached_laser_scan_.clear();
                    cached_laser_scan_.resize(laser_scan.size());
                    cached_laser_scan_.assign(laser_scan.begin(), laser_scan.end());

                    cv_.notify_one();
                    mtx_.unlock();
                }

                laser_scan.clear();

                is_got_one_frame = true;
                cross_frame_pt_idx = pos;
                break;
            }

            poelidar_point_t pt;
            pt.intensity = m1_pack.nodes[pos].confidence;
            pt.range = m1_pack.nodes[pos].distance;
            pt.angle = angle_now;

            laser_scan.push_back(pt);

            angle_pre = angle_now;
       }

       if (is_got_one_frame) {
            for (int pos = cross_frame_pt_idx; pos < M1_NODE_NUM_PER_PACK; pos++) {
                float angle_now = ((float)m1_pack.startAngle + angle_increment*pos) * 0.01f;
                angle_now = (angle_now >= 360.0f) ? (angle_now - 360.0f) : angle_now;

                poelidar_point_t pt;
                pt.intensity = m1_pack.nodes[pos].confidence;
                pt.range = m1_pack.nodes[pos].distance;
                pt.angle = angle_now;

                laser_scan.push_back(pt);
            }
        }

        pre_frame_last_pt_angle = m1_pack.endAngle * 0.01f;
    }

    data_thread_enbale_ = false;
    return;
}

void POELidarDriverImpl::data_thread_F1_(void)
{
    std::vector<poelidar_point_t>  laser_scan;

    int pre_block_idx = -1;

    while (data_thread_enbale_)
    {
        int block_idx = 0;
        int block_count = 0;

        HeadDataPackF1 * head_F1_pt;
        const int head_size = sizeof(HeadDataPackF1);

        const int buff_size = 4096;
        uint8_t   buff[buff_size];
        memset(buff, 0, buff_size);


        uint32_t startTime = get_ms_();
        uint32_t timeout = 1000U;
        uint32_t waitTime;

        while ((waitTime = get_ms_() - startTime) <= timeout) {
            int real_recv_size = socket_udp_->recvfrom((unsigned char*)buff, buff_size);
            if(real_recv_size > head_size){
                break;
            }
        }

        head_F1_pt = (HeadDataPackF1*)buff;
        block_idx = head_F1_pt->block_idx;
        block_count = head_F1_pt->count;


        if(block_idx < pre_block_idx ){
            mtx_.lock();
            cached_laser_scan_.clear();
            cached_laser_scan_.resize(laser_scan.size());
            cached_laser_scan_.assign(laser_scan.begin(), laser_scan.end());

            cv_.notify_one();
            mtx_.unlock();

            laser_scan.clear();
        }

        uint16_t * range_addr = nullptr;
        uint8_t * intensity_addr = nullptr;

        range_addr = (uint16_t *)(buff + head_size);
        intensity_addr = (uint8_t *)((uint8_t *)range_addr + block_count*2);

        float angle_increment = 270.0/ (8*block_count);

        for(int i=0; i<block_count;i++) {
            uint16_t range = range_addr[i];
            uint8_t intensity = intensity_addr[i];

            poelidar_point_t pt;
            pt.range = range*2;
            pt.intensity = intensity;
            pt.angle = -135.0 + laser_scan.size()*angle_increment;
            laser_scan.push_back(pt);
        }

        pre_block_idx = block_idx;
    }

    data_thread_enbale_ = false;

    cv_end_.notify_one();

    return;
}

bool POELidarDriverImpl::get_cmd_(
        uint8_t *cmd_buff, int &cmd_len, int type, uint8_t id,
        const uint8_t *params, int params_len,
        std::string value_str, bool value_is_number
        )
{
    memcpy((void *)cmd_buff, F1_CMD_HEAD, sizeof(F1_CMD_HEAD));
    cmd_len = sizeof(F1_CMD_HEAD);

    bool is_need_params = false;
    bool is_need_value = false;

    switch(type){
        case CPJ_F1_CMD_TYPE_GET: {
            is_need_params =  true;

            memcpy((void *)(cmd_buff + cmd_len), F1_CMD_TYPE_GET, sizeof(F1_CMD_TYPE_GET));
            cmd_len += sizeof(F1_CMD_TYPE_GET);
        } break;

        case CPJ_F1_CMD_TYPE_SET: {
            is_need_params =  true;
            is_need_value = true;

            memcpy((void *)(cmd_buff + cmd_len), F1_CMD_TYPE_SET, sizeof(F1_CMD_TYPE_SET));
            cmd_len += sizeof(F1_CMD_TYPE_SET);
        } break;

        case CPJ_F1_CMD_TYPE_START: {

            memcpy((void *)(cmd_buff + cmd_len), F1_CMD_TYPE_START, sizeof(F1_CMD_TYPE_START));
            cmd_len += sizeof(F1_CMD_TYPE_START);
        } break;

        case CPJ_F1_CMD_TYPE_STOP: {

            memcpy((void *)(cmd_buff + cmd_len), F1_CMD_TYPE_STOP, sizeof(F1_CMD_TYPE_STOP));
            cmd_len += sizeof(F1_CMD_TYPE_STOP);
        } break;

        case CPJ_F1_CMD_TYPE_QUERY: {
            is_need_params =  true;

            memcpy((void *)(cmd_buff + cmd_len), F1_CMD_TYPE_QUERY, sizeof(F1_CMD_TYPE_QUERY));
            cmd_len += sizeof(F1_CMD_TYPE_QUERY);
        } break;

        case CPJ_F1_CMD_TYPE_REMBER: {
            memcpy((void *)(cmd_buff + cmd_len), F1_CMD_TYPE_REMBER, sizeof(F1_CMD_TYPE_REMBER));
            cmd_len += sizeof(F1_CMD_TYPE_REMBER);
        } break;

        case CPJ_F1_CMD_TYPE_REBOOT: {
            memcpy((void *)(cmd_buff + cmd_len), F1_CMD_TYPE_REBOOT, sizeof(F1_CMD_TYPE_REBOOT));
            cmd_len += sizeof(F1_CMD_TYPE_REBOOT);
        } break;

        default : {
            return false;
        } break;
    }

    if(is_need_params){
        memcpy((void *)(cmd_buff + cmd_len), F1_PARAMS_HEAD, sizeof(F1_PARAMS_HEAD));
        cmd_len += sizeof(F1_PARAMS_HEAD);

        memcpy((void *)(cmd_buff + cmd_len), params, params_len);
        cmd_len += params_len;
    }

    if(is_need_value){
        memcpy((void *)(cmd_buff + cmd_len), F1_VALUE_HEAD, sizeof(F1_VALUE_HEAD));
        cmd_len += sizeof(F1_VALUE_HEAD);

        if(!value_is_number) {
            uint8_t a = 34;
            memcpy((void *)(cmd_buff + cmd_len), &a, 1);
            cmd_len ++;
        }

        memcpy((void *)(cmd_buff + cmd_len), value_str.c_str(), strlen(value_str.c_str()));
        cmd_len += strlen(value_str.c_str());

        if(!value_is_number){
            uint8_t a = 34;
            memcpy((void *)(cmd_buff + cmd_len), &a, 1);
            cmd_len ++;
        }

    }

    if(is_need_params || is_need_value){
        uint8_t a = 125;
        memcpy((void *)(cmd_buff + cmd_len), &a, 1);
        cmd_len ++;
    }

    memcpy((void *)(cmd_buff + cmd_len), F1_ID_HEAD, sizeof(F1_ID_HEAD));
    cmd_len += sizeof(F1_ID_HEAD);

    std::string  id_str = std::to_string(id);
    memcpy((void *)(cmd_buff + cmd_len), id_str.c_str(), strlen(id_str.c_str()) );
    cmd_len += strlen(id_str.c_str());

    memcpy((void *)(cmd_buff + cmd_len), F1_CMD_END, sizeof(F1_CMD_END));
    cmd_len += sizeof(F1_CMD_END);

    return true;
}


bool POELidarDriverImpl::tcp_read_data_(uint8_t * read_buff, int read_size, int & real_read_size, int timeout_ms)
{
    uint32_t startTime = get_ms_();

    int recv_size = 0;
    int remain_size = read_size;
    int data_pos = 0;
    while ((get_ms_() - startTime) <= timeout_ms) {
        if(!socket_->waitfordata(timeout_ms)) {
            return false;
        }
        recv_size = socket_->recv((unsigned char*)(read_buff+data_pos), remain_size);
        data_pos = data_pos + recv_size;
        remain_size = remain_size - recv_size;

        real_read_size = read_size - remain_size;
        if(remain_size <= 0){
            return true;
        }
    }

    return false;
}

uint8_t POELidarDriverImpl::m1_cal_crc8_(const uint8_t *data, uint16_t data_len)
{
    uint8_t crc = 0;
    while (data_len--) {
        crc = kM1CrcTable[(crc ^ *data) & 0xff];
        data++;
    }
    return crc;
}

uint32_t POELidarDriverImpl::get_ms_()
{
    auto now = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    return ms.count();
}


}} //namespace cpj::poelidar
