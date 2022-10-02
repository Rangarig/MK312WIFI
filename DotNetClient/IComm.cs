namespace ButtWifiShock
{
    public interface IComm {

        /// <summary>
        /// Sets up a connection to the Device or returns with an exception
        /// </summary>
        public void connect();

        /// <summary>
        /// Queries the connected status of the comm connection
        /// </summary>
        /// <returns>true if a successful connection has been set up</returns>
        public bool isConnected();

        /// <summary>
        /// Closes the connection to the device
        /// </summary>
        public void close();

        /// <summary>
        /// Reads byte(s) from the Socket
        /// </summary>
        /// <param name="timeout">How long in ms to wait until we give up on recieving a byte</param>
        /// <param name="buffer">The buffer to read the bytes into, length determines how many bytes are read</param>
        /// <returns>The byte that was read</returns>
        public void readBytes(byte[] buffer,long timeout);

        /// <summary>
        /// Reads byte(s) from the Socket
        /// </summary>
        /// <returns>The byte that was read</returns>
        public void readBytes(byte[] buffer) {
            readBytes(buffer,1000); // The defaut timeout waiting for an answer
        }


        /// <summary>
        /// Writes a number of bytes into the socket
        /// </summary>
        /// <param name="buffer">The data to be send, the length of the buffer will be sent</param>
        public void writeBytes(byte[] buffer);


    }

}