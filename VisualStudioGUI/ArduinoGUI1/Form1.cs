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
        string[] SplitData = new string[7];
        string xLinearRaw = "0";
        string yLinearRaw = "0";
        string SamplingRaw = "0";
        string xKalmanRaw = "0";
        string yKalmanRaw = "0";
        string RangeFLRaw = "0";
        string RangeFRRaw = "0";
        string RangeRRRaw = "0";
        string RangeRLRaw = "0";

        double radi = 0;
        double degree = 0;
        double radi2 = 0;
        double degree2 = 0;

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
            chart1.Series["Linear"].ChartType =
                SeriesChartType.Polar;
            chart1.Series["Linear"].Color = Color.Blue;
            chart1.Series["Kalman"].ChartType =
                SeriesChartType.Polar;
            chart1.Series["Kalman"].Color = Color.Green;
            //chart1.Series["Robot"].ChartType =
              //  SeriesChartType.FastPoint;
            chart1.Series["Robot"].Points.AddXY(0, 0);
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
                    SamplingRaw = SplitData[0];                 
                    RangeFLRaw = SplitData[1];
                    RangeFRRaw = SplitData[2];
                    RangeRRRaw = SplitData[3];
                    RangeRLRaw = SplitData[4];
                    xKalmanRaw = SplitData[5];
                    yKalmanRaw = SplitData[6];
                    xLinearRaw = SplitData[7];
                    yLinearRaw = SplitData[8];

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

            Plots.Add(new plot
            {
                SamplingFreq = float.Parse(SamplingRaw, CultureInfo.InvariantCulture.NumberFormat),
                xPosLinear = float.Parse(xLinearRaw, CultureInfo.InvariantCulture.NumberFormat),
                yPosLinear = float.Parse(yLinearRaw, CultureInfo.InvariantCulture.NumberFormat),
                xPosKalman = float.Parse(xKalmanRaw, CultureInfo.InvariantCulture.NumberFormat),
                yPosKalman = float.Parse(yKalmanRaw, CultureInfo.InvariantCulture.NumberFormat),
                RangeFL = float.Parse(RangeFLRaw, CultureInfo.InvariantCulture.NumberFormat),
                RangeFR = float.Parse(RangeFRRaw, CultureInfo.InvariantCulture.NumberFormat),
                RangeRR = float.Parse(RangeRRRaw, CultureInfo.InvariantCulture.NumberFormat),
                RangeRL = float.Parse(RangeRLRaw, CultureInfo.InvariantCulture.NumberFormat),
            });

     

            foreach (var s in Plots)
            {
                radi = Math.Sqrt(Math.Pow(s.xPosKalman,2) + Math.Pow(s.yPosKalman,2));
                radi2 = Math.Sqrt(Math.Pow(s.xPosLinear, 2) + Math.Pow(s.yPosLinear, 2));
                degree = Math.Atan2(s.yPosKalman, s.xPosKalman);
                degree = -1*(degree * 360 / (2 * Math.PI)-90);
                degree2 = Math.Atan2(s.yPosLinear, s.xPosLinear);
                degree2 = -1*(degree2 * 360 / (2 * Math.PI)-90);
                chart1.Series["Linear"].Points.Clear();
                //chart1.Series["Linear"].Points.AddXY(degree2, radi2);
                chart1.Series["Kalman"].Points.Clear();
                chart1.Series["Kalman"].Points.AddXY(degree, radi);
                label7.Text = "(" + s.xPosKalman.ToString("0.00") + ", " + s.yPosKalman.ToString("0.00") + ")";
                label13.Text = "(" + radi.ToString("0.00") + ", " + degree.ToString("0.00") + "°)";
                label12.Text = s.SamplingFreq.ToString("0.00");
            }
            Plots.Clear();

            label8.Text = RangeFLRaw;
            label9.Text = RangeFRRaw;
            label10.Text = RangeRRRaw;
            label11.Text = RangeRLRaw;

        }

        private void log(object sender, EventArgs e)
        {
            if (IsLogging)
            {
                Logger.Add(new plot
                {
                    SamplingFreq = float.Parse(SamplingRaw, CultureInfo.InvariantCulture.NumberFormat),
                    xPosLinear = float.Parse(xLinearRaw, CultureInfo.InvariantCulture.NumberFormat),
                    yPosLinear = float.Parse(yLinearRaw, CultureInfo.InvariantCulture.NumberFormat),
                    xPosKalman = float.Parse(xKalmanRaw, CultureInfo.InvariantCulture.NumberFormat),
                    yPosKalman = float.Parse(yKalmanRaw, CultureInfo.InvariantCulture.NumberFormat),
                    RangeFL = float.Parse(RangeFLRaw, CultureInfo.InvariantCulture.NumberFormat),
                    RangeFR = float.Parse(RangeFRRaw, CultureInfo.InvariantCulture.NumberFormat),
                    RangeRR = float.Parse(RangeRRRaw, CultureInfo.InvariantCulture.NumberFormat),
                    RangeRL = float.Parse(RangeRLRaw, CultureInfo.InvariantCulture.NumberFormat),
                });
            }
            /*   if (Logger.Count == 500)
               {
                   Filename = txFileName.Text;
                   btnLogging.Text = "Start Logging";
                   TextWriter tw = new StreamWriter("../../../../../TestingData/" + Filename + ".txt");
                   foreach (var s in Logger)
                       tw.WriteLine(s.SamplingFreq.ToString("0.00", new CultureInfo("en-US")) + "," + s.RangeFL.ToString("0.00", new CultureInfo("en-US")) +
                           "," + s.RangeFR.ToString("0.00", new CultureInfo("en-US")) + "," + s.RangeRR.ToString("0.00", new CultureInfo("en-US")) +
                           "," + s.RangeRL.ToString("0.00", new CultureInfo("en-US")) + "," + s.xPosLinear.ToString("0.00", new CultureInfo("en-US")) +
                           "," + s.yPosLinear.ToString("0.00", new CultureInfo("en-US")) + "," + s.xPosKalman.ToString("0.00", new CultureInfo("en-US")) +
                           "," + s.yPosKalman.ToString("0.00", new CultureInfo("en-US")));
                   tw.Close();
                   IsLogging = false;
                   Logger.Clear();
               }*/
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
                if (portName == "COM7")
                {
                    sp.BaudRate = 115200;
                }else
                {
                    sp.BaudRate = 115200;
                }
                try
                {
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
            public double SamplingFreq { get; set; }
            public double xPosLinear { get; set; }
            public double yPosLinear { get; set; }
            public double xPosKalman { get; set; }
            public double yPosKalman { get; set; }
            public double RangeFL { get; set; }
            public double RangeFR { get; set; }
            public double RangeRR { get; set; }
            public double RangeRL { get; set; }
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
                Filename = txFileName.Text;
                TextWriter tw = new StreamWriter("../../../../../TestingData/" + Filename + ".txt");
                foreach (var s in Logger)
                    tw.WriteLine(s.SamplingFreq.ToString("0.00", new CultureInfo("en-US")) + "," + s.RangeFL.ToString("0.00", new CultureInfo("en-US")) +
                        "," + s.RangeFR.ToString("0.00", new CultureInfo("en-US")) + "," + s.RangeRR.ToString("0.00", new CultureInfo("en-US")) +
                        "," + s.RangeRL.ToString("0.00", new CultureInfo("en-US")) + "," + s.xPosLinear.ToString("0.00", new CultureInfo("en-US")) +
                        "," + s.yPosLinear.ToString("0.00", new CultureInfo("en-US")) + "," + s.xPosKalman.ToString("0.00", new CultureInfo("en-US")) +
                        "," + s.yPosKalman.ToString("0.00", new CultureInfo("en-US"))
                        );
                tw.Close();
                IsLogging = false;
            }
        }

    }

}
