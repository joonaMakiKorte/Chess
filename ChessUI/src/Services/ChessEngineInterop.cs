using System;
using System.Runtime.InteropServices;
using System.Text.Json;

namespace Chess
{
    class ChessEngineInterop
    {
        private const string DllName = "ChessEngine.dll"; // DLL name
        private const int JsonBufferSize = 256; // Bytes to allocate for json output buffer

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr CreateBoard();

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void DestroyBoard(IntPtr board);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern ulong ValidMoves(IntPtr board, int square);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void MakeMove(IntPtr board, int source, int target, char promotion);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void MakeBestMove(IntPtr board, int depth, bool white);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        private static extern void GetBoardJSON(IntPtr board, IntPtr output, int size);

        // Define a class to hold the parsed JSON board data
        public class BoardStatusInfo
        {
            public string Move { get; set; }
            public string State { get; set; }
            public string Fen { get; set; }
        }

        // Get BoardStatusInfo of current board state
        public static BoardStatusInfo GetBoardStatus(IntPtr board)
        {
            // Check for invalid board pointer early
            if (board == IntPtr.Zero)
            {
                Console.Error.WriteLine("Error: GetBoardStatus called with null board pointer.");
                return null;
            }

            IntPtr buffer = IntPtr.Zero; // Initialize to zero is good practice
            try
            {
                // Allocate unmanaged memory for the buffer
                buffer = Marshal.AllocHGlobal(JsonBufferSize);

                // Call the C++ function to fill the buffer with JSON
                GetBoardJSON(board, buffer, JsonBufferSize);

                // Convert the C-style string (ANSI assumed based on PtrToStringAnsi) from the buffer to a C# string
                string jsonString = Marshal.PtrToStringAnsi(buffer);

                // --- JSON Deserialization ---
                if (string.IsNullOrEmpty(jsonString))
                {
                    Console.Error.WriteLine("Error: Engine returned an empty or null JSON string.");
                    return null;
                }

                try
                {
                    // Deserialize the JSON string into our record/class
                    // Using System.Text.Json
                    var options = new JsonSerializerOptions
                    {
                        PropertyNameCaseInsensitive = true // Makes parsing flexible regarding "move" vs "Move" etc.
                    };
                    BoardStatusInfo statusInfo = JsonSerializer.Deserialize<BoardStatusInfo>(jsonString, options);

                    // Optional: Add more validation if needed (e.g., check if Fen looks valid)
                    if (statusInfo == null || statusInfo.Fen == null || statusInfo.State == null || statusInfo.Move == null)
                    {
                        Console.Error.WriteLine($"Error: Failed to deserialize JSON completely or received null fields. JSON: {jsonString}");
                        return null;
                    }

                    return statusInfo;
                }
                catch (JsonException jsonEx)
                {
                    Console.Error.WriteLine($"Error parsing JSON from engine: {jsonEx.Message}. Raw JSON: '{jsonString}'");
                    return null; // Return null on parsing error
                }
            }
            catch (DllNotFoundException dllEx)
            {
                Console.Error.WriteLine($"Error: Engine DLL not found '{DllName}'. {dllEx.Message}");
                // Maybe rethrow or handle application-wide
                throw; // Rethrow for higher-level handling
            }
            catch (Exception ex) // Catch other potential interop or memory errors
            {
                Console.Error.WriteLine($"An unexpected error occurred during engine interop: {ex.Message}");
                return null;
            }
            finally
            {
                // --- IMPORTANT: Always free the allocated unmanaged memory ---
                if (buffer != IntPtr.Zero)
                {
                    Marshal.FreeHGlobal(buffer);
                }
            }
        }
    }
}
