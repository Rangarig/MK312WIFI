using MK312WifiLibDotNet;
using RexLabsWifiShock;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;


namespace DotNetAdapter
{
    class Program
    {
        static void Main(string[] args)
        {
            Program p = new Program();
            p.Start();
        }

        // Start is called before the first frame update

        void Start()

        {
            Console.WriteLine("Looking for Device...");
            //WifiComm comm = new WifiComm(); // Use the WIFI connection
            SerialComm serComm = new SerialComm("COM3"); // Use Serial Connection, please keep in mind that will not work with non encryption.
            MK312Device mk312Device = new MK312Device(serComm, true, true);

            mk312Device.connect();
            Console.WriteLine("Connected.");

            double position = 0;
            double movement = 0.1;
            mk312Device.setMode(MK312Constants.Mode.Stroke);
            Console.WriteLine("Mode Read:" + mk312Device.readMode());

            mk312Device.writeToDisplay("RexLabs");
            mk312Device.initializeChannels();
            while (true) {
                Thread.Sleep(100);

                mk312Device.setEncryptionKey(0x04);

                position += movement;
                if (position < 0) {
                    position = 0;
                    movement = -movement;
                }
                if (position > 1) {
                    position = 1;
                    movement = -movement;
                }
                Console.WriteLine(movement + " " + position);
                mk312Device.setChannelALevel(position);
                mk312Device.setChannelBLevel(position);
            }

            mk312Device.disconnect();

        }

    }


}
