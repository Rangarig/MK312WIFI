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
            WifiComm comm = new WifiComm();
            MK312Device mk312functions = new MK312Device(comm, false,true);

            mk312functions.connect();
            Console.WriteLine("Connected.");

            double position = 0;
            double movement = 0.1;
            mk312functions.setMode(MK312Constants.Mode.Stroke);
            Console.WriteLine("Mode Read:" + mk312functions.readMode());

            mk312functions.writeToDisplay("RexLabs");
            mk312functions.initializeChannels();
            while (true) {
                Thread.Sleep(100);

                mk312functions.setEncryptionKey(0x04);

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
                mk312functions.setChannelALevel(position);
                mk312functions.setChannelBLevel(position);
            }

            mk312functions.disconnect();

        }

    }


}
