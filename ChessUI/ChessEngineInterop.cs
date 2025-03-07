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
        public static extern bool ValidateMove(IntPtr board, string move);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void GetBoardState(IntPtr board, IntPtr output, int size);

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
    }
}
