using UnityEngine;

public class LiDARTest : MonoBehaviour
{
    void Update()
    {
        while (LiDARService.LiDAR_data.TryDequeue(out string line))
            UnityEngine.Debug.Log(line);
    }
}