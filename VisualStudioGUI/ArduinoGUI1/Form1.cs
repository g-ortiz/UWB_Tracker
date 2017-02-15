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
        string xRaw = "0";
        string yRaw = "0";
        string xFilterRaw = "0";
        string yFilterRaw = "0";
        string SamplingRaw = "0";
        string Direction = "0";
        string Forward = "0";
        string RangeFL = "0";
        string RangeFR = "0";
        string RangeRR = "0";
        string RangeRL = "0";
        double xPlot = 0;
        double yPlot = 0;
        string ActualPort = "";
        bool IsConnected = false;
        bool IsLogging = false;
        string Filename = null;
        PointF Intersection1, Intersection2;
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
            chart1.Series["Circles"].ChartType =
                SeriesChartType.FastPoint;
            chart1.Series["Circles"].Color = Color.Green;
            chart1.Series["Robot"].ChartType =
                SeriesChartType.FastPoint;
            chart1.Series["Robot"].Points.AddXY(string.Empty, 0);
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
                    Direction = SplitData[5];
                    Forward = SplitData[6];
                    RangeFL = SplitData[7];
                    RangeFR = SplitData[8];
                    RangeRR = SplitData[9];
                    RangeRL = SplitData[10];
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
            xPlot = float.Parse(xFilterRaw, CultureInfo.InvariantCulture.NumberFormat);
            yPlot = float.Parse(yFilterRaw, CultureInfo.InvariantCulture.NumberFormat);
            chart1.Series["Target"].Points.Clear();
            chart1.Series["Target"].Points.AddXY(xPlot, yPlot);
            label1.Text = "Position: (" + xPlot.ToString("0.00") + ", " + yPlot.ToString("0.00") + ")";
            label5.Text = "Distance (RL): " + yRaw + "cm";
            label4.Text = "Sampling: " + SamplingRaw;

            flrange.Text = "RangeFL: " + RangeFL;
            frrange.Text = "RangeFR: " + RangeFR;
            rrrange.Text = "RangeRR: " + RangeRR;
            rlrange.Text = "RangeRL: " + RangeRL;
            float radius0 = float.Parse(RangeFL, CultureInfo.InvariantCulture.NumberFormat);
            float radius1 = float.Parse(RangeFR, CultureInfo.InvariantCulture.NumberFormat);

            if (Forward == "1")
            {
                front.BackColor = Color.Green;
                back.BackColor = Color.White;
            }
            else if (Forward == "2")
            {
                front.BackColor = Color.White;
                back.BackColor = Color.Green;
            }else
            {
                front.BackColor = Color.White;
                back.BackColor = Color.White;
            }
            if (Direction == "1")
            {
                right.BackColor = Color.Green;
                left.BackColor = Color.White;
            }
            else if (Direction == "2")
            {
                right.BackColor = Color.White;
                left.BackColor = Color.Green;
            }else
            {
                right.BackColor = Color.White;
                left.BackColor = Color.White;
            }

            FindCircleCircleIntersections(
                0, 0, radius0, 31, 0, radius1,
                out Intersection1, out Intersection2);
            chart1.Series["Circles"].Points.Clear();
            chart1.Series["Circles"].Points.AddXY(Intersection1.X, Intersection1.Y);
            chart1.Series["Circles"].Points.AddXY(Intersection2.X, Intersection2.Y);
            Plots.Clear();
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
                    TxPower = float.Parse(Direction, CultureInfo.InvariantCulture.NumberFormat),
                    TxRxQ = float.Parse(Forward, CultureInfo.InvariantCulture.NumberFormat),
                });
            }
            if (Logger.Count == 500)
            {
                Filename = txFileName.Text;
                btnLogging.Text = "Start Logging";
                TextWriter tw = new StreamWriter("../../../../../TestingData/" + Filename + ".txt");
                foreach (var s in Logger)
                    tw.WriteLine(s.xPosRaw.ToString("0.00", new CultureInfo("en-US")) + "," + s.yPosRaw.ToString("0.00", new CultureInfo("en-US")) +
                        "," + s.xPosFilter.ToString("0.00", new CultureInfo("en-US")) + "," + s.yPosFilter.ToString("0.00", new CultureInfo("en-US")) +
                        "," + s.SamplingFreq.ToString("0.00", new CultureInfo("en-US")) +
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
                TextWriter tw = new StreamWriter("../../../../../TestingData/" + Filename + ".txt");
                foreach (var s in Logger)
                    tw.WriteLine(s.xPosRaw.ToString("0.00", new CultureInfo("en-US")) + "," + s.yPosRaw.ToString("0.00", new CultureInfo("en-US")) +
                        "," + s.xPosFilter.ToString("0.00", new CultureInfo("en-US")) + "," + s.yPosFilter.ToString("0.00", new CultureInfo("en-US")) +
                        "," + s.SamplingFreq.ToString("0.00", new CultureInfo("en-US")) +
                        "," + s.TxPower.ToString("0.00", new CultureInfo("en-US")) + "," + s.TxRxQ.ToString("0.00", new CultureInfo("en-US")));
                tw.Close();
                IsLogging = false;
            }
        }

        // Find the points where the two circles intersect.
        private int FindCircleCircleIntersections(
            float cx0, float cy0, float radius0,
            float cx1, float cy1, float radius1,
            out PointF intersection1, out PointF intersection2)
        {
            // Find the distance between the centers.
            float dx = cx0 - cx1;
            float dy = cy0 - cy1;
            double dist = Math.Sqrt(dx * dx + dy * dy);

            // See how many solutions there are.
            if (dist > radius0 + radius1)
            {
                // No solutions, the circles are too far apart.
                intersection1 = new PointF(float.NaN, float.NaN);
                intersection2 = new PointF(float.NaN, float.NaN);
                return 0;
            }
            else if (dist < Math.Abs(radius0 - radius1))
            {
                // No solutions, one circle contains the other.
                intersection1 = new PointF(float.NaN, float.NaN);
                intersection2 = new PointF(float.NaN, float.NaN);
                return 0;
            }
            else if ((dist == 0) && (radius0 == radius1))
            {
                // No solutions, the circles coincide.
                intersection1 = new PointF(float.NaN, float.NaN);
                intersection2 = new PointF(float.NaN, float.NaN);
                return 0;
            }
            else
            {
                // Find a and h.
                double a = (radius0 * radius0 -
                    radius1 * radius1 + dist * dist) / (2 * dist);
                double h = Math.Sqrt(radius0 * radius0 - a * a);

                // Find P2.
                double cx2 = cx0 + a * (cx1 - cx0) / dist;
                double cy2 = cy0 + a * (cy1 - cy0) / dist;

                // Get the points P3.
                intersection1 = new PointF(
                    (float)(cx2 + h * (cy1 - cy0) / dist),
                    (float)(cy2 - h * (cx1 - cx0) / dist));
                intersection2 = new PointF(
                    (float)(cx2 - h * (cy1 - cy0) / dist),
                    (float)(cy2 + h * (cx1 - cx0) / dist));

                // See if we have 1 or 2 solutions.
                if (dist == radius0 + radius1) return 1;
                return 2;
            }
        }

    }

}
