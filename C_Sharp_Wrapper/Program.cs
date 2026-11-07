using System.Diagnostics;

class Program
{
    static async Task<int> Main(string[] args) {
        const string exeName = "LiDAR_getData.exe";   // Linux binary name: LiDAR_getData

        string? ip = null;
        int? port = null;
        bool simulate = false;

        for (int i = 0; i < args.Length; i++)
        {
            switch (args[i])
            {
                case "-h" or "--help":
                    Console.WriteLine("Usage: LiDAR_app [options]");
                    Console.WriteLine("Options:");
                    Console.WriteLine("  -h, --help          Show this help message and exit");
                    Console.WriteLine("      --ip <IP>       Device IP address");
                    Console.WriteLine("      --port <PORT>   Device port number");
                    Console.WriteLine("      --simulate      Simulate LiDAR data output");
                    return 0;

                case "--ip":
                    if (++i >= args.Length) {
                        Console.Error.WriteLine("Error: --ip requires a value.");
                        return 1;
                    }
                    ip = args[i];
                    break;

                case "--port":
                    if (++i >= args.Length || !int.TryParse(args[i], out int p) || p < 1 || p > 65535) {
                        Console.Error.WriteLine("Error: --port requires a number between 1 and 65535.");
                        return 1;
                    }
                    port = p;
                    break;

                case "--simulate":
                    simulate = true;
                    break;

                default:
                    Console.Error.WriteLine($"Error: unknown option '{args[i]}'.");
                    Console.WriteLine("Usage: LiDAR_app [options]");
                    Console.WriteLine("Options:");
                    Console.WriteLine("  -h, --help          Show this help message and exit");
                    Console.WriteLine("      --ip <IP>       Device IP address");
                    Console.WriteLine("      --port <PORT>   Device port number");
                    Console.WriteLine("      --simulate      Simulate LiDAR data output");
                    return 1;
            }
        }

        string exePath = Path.Combine(AppContext.BaseDirectory, exeName);
        if (!File.Exists(exePath)) {
            Console.Error.WriteLine($"LiDAR_getData executable not found at:  {exePath}");
            Console.Error.WriteLine("Build it and copy it next to this executable.");
            return 1;
        }

        var psi = new ProcessStartInfo {
            FileName = exePath,
            UseShellExecute = false,
            RedirectStandardOutput = true,
            RedirectStandardError = true
        };
        if (ip != null)     psi.ArgumentList.Add($"--ip {ip}");
        if (port != null)   psi.ArgumentList.Add($"--port {port}");
        if (simulate)       psi.ArgumentList.Add("--simulate");

        using var LiDAR_getData_process = Process.Start(psi)!;
        Task<string> stderrTask = LiDAR_getData_process.StandardError.ReadToEndAsync();

        string? LiDAR_getData_process_stdOut;
        while ((LiDAR_getData_process_stdOut = await LiDAR_getData_process.StandardOutput.ReadLineAsync()) != null) {

            // Process `LiDAR_getData_process_stdOut` as required
            // format as stringified numpy array: [[polar_r], [polar_theta], [polar_intensity]]
            Console.WriteLine(LiDAR_getData_process_stdOut);

        }

        await LiDAR_getData_process.WaitForExitAsync();
        string LiDAR_getData_process_stdErr = await stderrTask;
        if (LiDAR_getData_process_stdErr.Length > 0) Console.Error.WriteLine(LiDAR_getData_process_stdErr);

        return LiDAR_getData_process.ExitCode;
    }
}