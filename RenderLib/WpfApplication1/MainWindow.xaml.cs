using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Windows.Interop; //HwndHost
using System.Runtime.InteropServices;
using System.Windows.Forms;

using System.Windows.Forms.Integration;

[StructLayout(LayoutKind.Sequential)]
public struct RECT
{
    public int Left;        // x position of upper-left corner
    public int Top;         // y position of upper-left corner
    public int Right;       // x position of lower-right corner
    public int Bottom;      // y position of lower-right corner
}

namespace WPF_OpenGL_Test
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>

    public unsafe partial class MainWindow : Window
    {
        [DllImport(@"D:\Download\OpenGL Test\WPF_OpenGL_Test\WPF_OpenGL_Test\OpenGLShow_DLL.dll", EntryPoint = "TryOpenGLShow", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        public unsafe extern static void TryOpenGLShow(IntPtr hWndcsharp);

        [DllImport("user32.dll")]
        [return: MarshalAs(UnmanagedType.Bool)]
        static extern bool GetWindowRect(HandleRef hWnd, out RECT lpRect);

        // 3D Reconn
        [DllImport(@"D:\CarbonMed\System\Bin\RenderLib.dll", EntryPoint = "Initial3DReconResource", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        public unsafe extern static int Initial3DReconResource(float* dicominfo, float* dicomdata, int framecount, IntPtr hWndcsharp);
        [DllImport(@"D:\CarbonMed\System\Bin\RenderLib.dll", EntryPoint = "Release3DReconResource", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        public extern static void Release3DReconResource();
        [DllImport(@"D:\CarbonMed\System\Bin\RenderLib.dll", EntryPoint = "Call3DRecon", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        public unsafe extern static void Call3DRecon(float* maskata, float* maskinfo);

        public MainWindow()
        {
            InitializeComponent();

            //WindowsFormsHost.EnableWindowsFormsInterop();
        }

        private void button_Click(object sender, RoutedEventArgs e)
        {
            IntPtr hwnd_3D = IntPtr.Zero;

            // 方法 1
            //             var hwndSource = PresentationSource.FromVisual(this) as HwndSource;
            //             hwnd_3D = hwndSource.Handle;

            // 方法 2
            //             hwnd_3D = new WindowInteropHelper(this).Handle;
            //             hwnd_3D = new WindowInteropHelper(WPF_OpenGL_Window).Handle;

            // 方法 3
            hwnd_3D = wfh_3D_screen.Handle;

            // 通过句柄获得窗口的大小，确定我们是否传错了窗口
            /*RECT rct;
            if (!GetWindowRect(new HandleRef(this, hwnd_3D), out rct))
                return;
            int x = rct.Left;
            int y = rct.Top;
            int width = rct.Right - rct.Left + 1;
            int height = rct.Bottom - rct.Top + 1;*/
            // 窗口尺寸是对的，宽544，高622

            int curSequenceCount = 75;
            float[] ReconPara = new float[75 * 7];
            float[] SequenceData = new float[512 * 512 * 200];

            Array.Clear(ReconPara, 0, ReconPara.Length);
            Array.Clear(SequenceData, 0, SequenceData.Length);

            // 传入句柄
            if (hwnd_3D != IntPtr.Zero)
            {
                fixed (float* seqDICOMInfo = &ReconPara[0])
                {
                    fixed (float* seqDICOMData = &SequenceData[0])
                    {
                        int ini3d = Initial3DReconResource(seqDICOMInfo, seqDICOMData, curSequenceCount, hwnd_3D);
                    }
                }

                float[] BWSequence_20 = new float[512 * 512 * 20];
                Array.Clear(BWSequence_20, 0, BWSequence_20.Length);

                float[] FrameInfo = new float[3];
                FrameInfo[0] = 1.0f;
                FrameInfo[1] = 0.5f;
                FrameInfo[2] = 20.0f;
                fixed (float* OrganImageData = &BWSequence_20[0])
                {
                    fixed (float* ReconFrameInfo = &FrameInfo[0])
                    {
                        Call3DRecon(OrganImageData, ReconFrameInfo);
                    }
                }

            }
            //TryOpenGLShow(hwnd_3D);
        }
    }
}
