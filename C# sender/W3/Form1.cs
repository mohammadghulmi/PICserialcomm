using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO.Ports;

namespace W3
{
    public partial class Form1 : Form
    {
        int Count = 0;
        int Count2 = 0;
        bool isOpen = false;
        bool IsMaster = true;
        const char EndChar = '\f'; //choose any character, for example \r, any character
        string RecvStr = "";
        public Form1()
        {
            InitializeComponent();
            Count = 0;
            Count2 = 0;
            IsMaster = true;
            isOpen = false;
            RecvStr = "";
        }

        private void button1_Click(object sender, EventArgs e)
        {
            if (isOpen) { serialPort1.Close(); isOpen = false; }
            serialPort1.PortName = comboBox1.SelectedItem.ToString();
            //serialPort1.BaudRate = Int32.Parse(baudComboBox.SelectedItem.ToString());
            //if (!IsMaster) serialPort1.PortName = "COM2";
            string tos = textBox1.Text;
            serialPort1.Open();
            serialPort1.Write(tos);
            
        }

        private void OReceived(object sender, System.IO.Ports.SerialDataReceivedEventArgs e)
        {
           
            string ss = serialPort1.ReadExisting();
            textBox1.AppendText("\n"+ss +"\r\n");
            bool terminated = (ss.IndexOf(EndChar) > 0);
            RecvStr += ss;
            if(terminated) timer2.Stop();//should only stop this if the terminor is received
            if (!IsMaster  && terminated)
            {
                serialPort1.Write( RecvStr + "\n"); //ss);//Timer 1
               // RecvStr = "";
            }
            if(terminated) RecvStr = "";
        }                                                                                      

        private void OnTick(object sender, EventArgs e)
        {
            RecvStr = "";//reinitialize receive string
            serialPort1.Write("T" + Count++ + EndChar);//Timer 1
            timer2.Start();//start timeout timer for master
        }

        private void OnTick2(object sender, EventArgs e)
        {
            timer2.Stop();
            textBox1.AppendText("Timeout"+(Count -1)+"\r\n");
        }

        private void checkBox1_CheckedChanged(object sender, EventArgs e)
        {
            //timeout
            IsMaster = !IsMaster;
        }

        private void button2_Click(object sender, EventArgs e)
        {
            string[] ports = SerialPort.GetPortNames();

            // textBox1.AppendText("The following serial ports were found:\r\n");

            // Display each port name to the console.
            //portsComboBox.Items.Clear();
            foreach (string port in ports)
            {
                //textBox1.AppendText(port +"\r\n");
                comboBox1.Items.Insert(0, port);
            }
            if (ports.Length > 0) comboBox1.SelectedIndex = 0;
            //statusTextBox.Text = "Found " + ports.Length + " ports";

        }

        private void textBox1_TextChanged(object sender, EventArgs e)
        {

        }

        private void label2_Click(object sender, EventArgs e)
        {

        }
    }
}
