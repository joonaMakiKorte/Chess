using System;
using System.Runtime.InteropServices;

namespace Chess
{
    class ChessEngineInterop
    {
        private const string DllName = "ChessEngine.dll"; // Load dll

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr CreateBoard();

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void DestroyBoard(IntPtr board);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern ulong ValidMoves(IntPtr board, int square);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void MakeMove(IntPtr board, int source, int target);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        private static extern void GetBoardState(IntPtr board, IntPtr output, int size);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr GetDebugMessage(IntPtr board, IntPtr output, int size);

        // Helper method to get board state as a string
        public static string GetBoardStateString(IntPtr board)
        {
            int bufferSize = 100; // Adjust buffer size as needed
            IntPtr buffer = Marshal.AllocHGlobal(bufferSize);

            GetBoardState(board, buffer, bufferSize);

            string fen = Marshal.PtrToStringAnsi(buffer);
            Marshal.FreeHGlobal(buffer);

            return fen;
        }

        // Helper to get debug messages as string
        public static string GetDebugMessageString(IntPtr board)
        {
            int bufferSize = 100;
            IntPtr buffer = Marshal.AllocHGlobal(bufferSize);

            GetDebugMessage(board, buffer, bufferSize);

            string message = Marshal.PtrToStringAnsi(buffer);
            Marshal.FreeHGlobal(buffer);

            return message;
        }
    }
}
