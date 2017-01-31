using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Windows.Forms.DataVisualization.Charting;
using System.Windows.Forms;
using System.IO.Ports;
using System.Globalization;
using System.IO;


namespace ArduinoGUI1
{
    public partial class Form1 : Form
    {
        SerialPort sp;
        string RawData = "0";
        string[] SplitData = new string[8]; 
        string xRaw = "0";
        string yRaw = "0";
        string xFilterRaw = "0";
        string yFilterRaw = "0";
        string SamplingRaw = "0";
        string PowerRaw = "0";
        string QualityRaw = "0";
        string TimeRaw = "0";
        double xPlot = 0;
        double yPlot = 0;
        string ActualPort = "";
        bool IsConnected = false;
        bool IsLogging = false;
        string Filename = null;
        List<plot> Plots = new List<plot>();
        List<plot> Logger = new List<plot>();

        public Form1()
        {
            InitializeComponent();
            //GenerateDataGrid();
        }



        private void Form1_Load(object sender, EventArgs e)
        {
            var ports = SerialPort.GetPortNames();
            cmbSerialPorts.DataSource = ports;
            chart1.Series["Target"].ChartType =
                SeriesChartType.FastPoint;
            chart1.Series["Target"].Color = Color.Red;
            chart1.Series["Robot"].ChartType =
                SeriesChartType.FastPoint;
            chart1.Series["Robot"].Points.AddXY(0, 0);
            chart1.Series["Robot"].Color = Color.Blue;
        }


        private void sp_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            if (IsConnected)
            {
                //var serialPort = (SerialPort)sender;
                //System.Threading.Thread.Sleep(500);
                try
                {

                    RawData = sp.ReadLine();
                    SplitData = RawData.Split(',');
                    xRaw = SplitData[0];
                    yRaw = SplitData[1];
                    xFilterRaw = SplitData[2];
                    yFilterRaw = SplitData[3];
                    SamplingRaw = SplitData[4];
                    PowerRaw = SplitData[5];
                    QualityRaw = SplitData[6];
                    TimeRaw = SplitData[7];
                    this.Invoke(new EventHandler(display));
                    this.Invoke(new EventHandler(log));
                }
                catch (Exception)
                {

                    sp.Close();
                }

            }
        }

        private void display(object sender, EventArgs e)
        {
            if (float.Parse(yRaw, CultureInfo.InvariantCulture.NumberFormat) > 0)
            {
                Plots.Add(new plot
                {
                    xPosRaw = float.Parse(xRaw, CultureInfo.InvariantCulture.NumberFormat),
                    yPosRaw = float.Parse(yRaw, CultureInfo.InvariantCulture.NumberFormat),
                    //ResponseTime = float.Parse(TimeRaw, CultureInfo.InvariantCulture.NumberFormat),
                    //TxPower = float.Parse(PowerRaw, CultureInfo.InvariantCulture.NumberFormat)
                });
            }


            Console.WriteLine(xRaw + " " + yRaw);
            if (Plots.Count == 20)
            {
                chart1.Series["Target"].Points.Clear();
                xPlot = 0;
                yPlot = 0;
                foreach (var p in Plots)
                {
                    xPlot += p.xPosRaw / 20;
                    yPlot += p.yPosRaw / 20;
                }
                chart1.Series["Target"].Points.AddXY(xPlot, yPlot);
                label1.Text = "Location: (" + xPlot.ToString("0.00") + ", " + yPlot.ToString("0.00") + ")";
                label4.Text = "Sampling: " + SamplingRaw;
                Plots.Clear();
            }
        }

        private void log(object sender, EventArgs e)
        {
            if (IsLogging)
            {
                Logger.Add(new plot
                {
                    xPosRaw = float.Parse(xRaw, CultureInfo.InvariantCulture.NumberFormat),
                    yPosRaw = float.Parse(yRaw, CultureInfo.InvariantCulture.NumberFormat),
                    xPosFilter = float.Parse(xFilterRaw, CultureInfo.InvariantCulture.NumberFormat),
                    yPosFilter = float.Parse(yFilterRaw, CultureInfo.InvariantCulture.NumberFormat),
                    SamplingFreq = float.Parse(SamplingRaw, CultureInfo.InvariantCulture.NumberFormat),
                    TxPower = float.Parse(PowerRaw, CultureInfo.InvariantCulture.NumberFormat),
                    TxRxQ = float.Parse(QualityRaw, CultureInfo.InvariantCulture.NumberFormat),
                    ResponseTime = float.Parse(TimeRaw, CultureInfo.InvariantCulture.NumberFormat)
                });
            }
            if (Logger.Count == 1000)
            {
                Filename = txFileName.Text;
                btnLogging.Text = "Start Logging";
                TextWriter tw = new StreamWriter("../../../../../TestingData/" + Filename + ".txt");
                foreach (var s in Logger)
                    tw.WriteLine(s.xPosRaw.ToString("0.00", new CultureInfo("en-US")) + "," + s.yPosRaw.ToString("0.00", new CultureInfo("en-US")) +
                        "," + s.xPosFilter.ToString("0.00", new CultureInfo("en-US")) + "," + s.yPosFilter.ToString("0.00", new CultureInfo("en-US")) +
                        "," + s.SamplingFreq.ToString("0.00", new CultureInfo("en-US")) + "," + s.ResponseTime.ToString("0.00", new CultureInfo("en-US")) +
                        "," + s.TxPower.ToString("0.00", new CultureInfo("en-US")) + "," + s.TxRxQ.ToString("0.00", new CultureInfo("en-US")));
                tw.Close();
                IsLogging = false;
                Logger.Clear();
            }
        }

        private void btnConnect_Click(object sender, EventArgs e)
        {
            if (cmbSerialPorts.SelectedIndex > -1)
            {
                Connect(cmbSerialPorts.SelectedItem.ToString());
            }
            else
            {
                MessageBox.Show("Please select a port first");
            }
        }


        private void Connect(string portName)
        {
            
            bool newport = portName.Equals(ActualPort, StringComparison.Ordinal);
            if (!IsConnected)
            { 
                sp = new SerialPort(portName);            
                try
                {
                    sp.BaudRate = 115200;
                    sp.Open();
                    ActualPort = portName;
                    sp.DataReceived += sp_DataReceived;
                    label2.Text = "Connected to " + ActualPort;
                    IsConnected = true;
                }
                catch (Exception ex)
                {
                    MessageBox.Show("Serial port not found",
                    "Error",
                    MessageBoxButtons.OK,
                    MessageBoxIcon.Error
                   );
                }
            } 
            if (!newport)
            {
                sp.Close();
                sp = new SerialPort(portName);
                try
                {
                    sp.BaudRate = 9600;
                    sp.Open();
                    ActualPort = portName;
                    sp.DataReceived += sp_DataReceived;
                    label2.Text = "Connected to " + ActualPort;
                    IsConnected = true;
                }
                catch (Exception ex)
                {
                    MessageBox.Show("Serial port not found",
                    "Error",
                    MessageBoxButtons.OK,
                    MessageBoxIcon.Error
                   );
                }
            }                     
        }

        private void button2_Click(object sender, EventArgs e)
        {
            IsConnected = false;
            sp.Close();
            label2.Text = "Disconnected";
        }

        private class plot
        {
            public double xPosRaw { get; set; }
            public double yPosRaw { get; set; }
            public double xPosFilter { get; set; }
            public double yPosFilter { get; set; }
            public double SamplingFreq { get; set; }
            public double TxPower { get; set; }
            public double TxRxQ { get; set; }
            public double ResponseTime { get; set; }
        }

        private void Form1_Closing(object sender, CancelEventArgs e)
        {
            if (IsConnected)
            {
                    sp.Close();
            }
        }

        private void btnLogging_Click(object sender, EventArgs e)
        {
            if (!IsLogging)
            {
                if (string.IsNullOrWhiteSpace(txFileName.Text))
                {
                    MessageBox.Show("Please write a filename");
                }
                else
                {
                    IsLogging = true;
                    btnLogging.Text = "Stop Logging";
                }

            }
            else
            {
                btnLogging.Text = "Start Logging";
                TextWriter tw = new StreamWriter("../../../../../TestingData/" + Filename +  ".txt");
                foreach (var s in Logger)
                    tw.WriteLine(s.xPosRaw.ToString("0.00", new CultureInfo("en-US")) + "," + s.yPosRaw.ToString("0.00", new CultureInfo("en-US")) +
                        "," + s.xPosFilter.ToString("0.00", new CultureInfo("en-US")) + "," + s.yPosFilter.ToString("0.00", new CultureInfo("en-US")) +
                        "," + s.SamplingFreq.ToString("0.00", new CultureInfo("en-US")) + "," + s.ResponseTime.ToString("0.00", new CultureInfo("en-US")) + 
                        "," + s.TxPower.ToString("0.00", new CultureInfo("en-US")) + "," + s.TxRxQ.ToString("0.00", new CultureInfo("en-US")));
                tw.Close();
                IsLogging = false;
            }
        }

    }

}
