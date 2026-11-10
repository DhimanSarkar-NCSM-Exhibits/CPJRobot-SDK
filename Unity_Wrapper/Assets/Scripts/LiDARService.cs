using System.Collections.Concurrent;
using System.Diagnostics;
using System.IO;
using UnityEngine;

public static class LiDARService
{
    public static string ip = "192.168.1.10"; // Defatult IP 192.168.1.10
    public static int port = 25168; // Default Port 25168
    public static bool simulate = false;

    public static readonly ConcurrentQueue<string> LiDAR_data = new ConcurrentQueue<string>();
    /*
        `LiDAR_datta` holds the current LiDARA data frame.
        Access this variable in any script as `line` variable,

        while (LiDARService.buffer.TryDequeue(out string line))
        {
            UnityEngine.Debug.Log(line);
            // FORMAT: [[polar_r], [polar_theta], [polar_intensity]]
        }
    */

    static Process p;

    [RuntimeInitializeOnLoadMethod(RuntimeInitializeLoadType.AfterSceneLoad)]
    static void Init() {
        if (p != null && !p.HasExited) return;   // already running

        #if UNITY_STANDALONE_WIN || UNITY_EDITOR_WIN
            const string exeName = "LiDAR_getData.exe";
        #else
            const string exeName = "LiDAR_getData";
        #endif

        var psi = new ProcessStartInfo {
            FileName = Path.Combine(Application.streamingAssetsPath, exeName),
            ArgumentList = {
                "--ip",     ip,
                "--port",   port.ToString()},
            UseShellExecute = false, CreateNoWindow = true,
            RedirectStandardOutput = true
        };
        if (simulate) psi.ArgumentList.Add("--simulate");

        // Forced Simulation Mode Activated for Debug and tesing
        psi.ArgumentList.Add("--simulate"); // Disable in prod.
        //////////////////////////////////////////////////////

        p = Process.Start(psi);
        p.OutputDataReceived += (s, e) => {
            if (e.Data == null) return;
            LiDAR_data.Clear();
            LiDAR_data.Enqueue(e.Data);
        };

        p.BeginOutputReadLine();

        Application.quitting += () => { if (!p.HasExited) p.Kill(); };
    }
}