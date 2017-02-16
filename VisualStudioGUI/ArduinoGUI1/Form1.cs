﻿using System;
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
        string xFrontRaw = "0";
        string yFrontRaw = "0";
        string xLeftRaw = "0";
        string yLeftRaw = "0";
        string xBackRaw = "0";
        string yBackRaw = "0";
        string xRightRaw = "0";
        string yRightRaw = "0";
        string xLinearRaw = "0";
        string yLinearRaw = "0";
        string SamplingRaw = "0";
        string Direction = "0";
        string Forward = "0";
        string RangeFLRaw = "0";
        string RangeFRRaw = "0";
        string RangeRRRaw = "0";
        string RangeRLRaw = "0";

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
            chart1.Series["Front"].ChartType =
                SeriesChartType.FastPoint;
            chart1.Series["Front"].Color = Color.Red;
            chart1.Series["Left"].ChartType =
                SeriesChartType.FastPoint;
            chart1.Series["Left"].Color = Color.Green;
            chart1.Series["Back"].ChartType =
                SeriesChartType.FastPoint;
            chart1.Series["Back"].Color = Color.Blue;
            chart1.Series["Right"].ChartType =
                SeriesChartType.FastPoint;
            chart1.Series["Right"].Color = Color.Brown;
            chart1.Series["Linear"].ChartType =
                SeriesChartType.FastPoint;
            chart1.Series["Linear"].Color = Color.Orange;
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
                    SamplingRaw = SplitData[2];
                    Direction = SplitData[3];
                    Forward = SplitData[4];
                    RangeFLRaw = SplitData[5];
                    RangeFRRaw = SplitData[6];
                    RangeRRRaw = SplitData[7];
                    RangeRLRaw = SplitData[8];
                    xFrontRaw = SplitData[9];
                    yFrontRaw = SplitData[10];
                    xLeftRaw = SplitData[11];
                    yLeftRaw = SplitData[12];
                    xBackRaw = SplitData[13];
                    yBackRaw = SplitData[14];
                    xRightRaw = SplitData[15];
                    yRightRaw = SplitData[16];
                    xLinearRaw = SplitData[17];
                    yLinearRaw = SplitData[18];

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
                xPosRaw = float.Parse(xRaw, CultureInfo.InvariantCulture.NumberFormat),
                yPosRaw = float.Parse(yRaw, CultureInfo.InvariantCulture.NumberFormat),
                SamplingFreq = float.Parse(SamplingRaw, CultureInfo.InvariantCulture.NumberFormat),
                Direction = float.Parse(Direction, CultureInfo.InvariantCulture.NumberFormat),
                Forward = float.Parse(Forward, CultureInfo.InvariantCulture.NumberFormat),
                xPosFront = float.Parse(xFrontRaw, CultureInfo.InvariantCulture.NumberFormat),
                yPosFront = float.Parse(yFrontRaw, CultureInfo.InvariantCulture.NumberFormat),
                xPosLeft = float.Parse(xLeftRaw, CultureInfo.InvariantCulture.NumberFormat),
                yPosLeft = float.Parse(yLeftRaw, CultureInfo.InvariantCulture.NumberFormat),
                xPosBack = float.Parse(xBackRaw, CultureInfo.InvariantCulture.NumberFormat),
                yPosBack = float.Parse(yBackRaw, CultureInfo.InvariantCulture.NumberFormat),
                xPosRight = float.Parse(xRightRaw, CultureInfo.InvariantCulture.NumberFormat),
                yPosRight = float.Parse(yRightRaw, CultureInfo.InvariantCulture.NumberFormat),
                xPosLinear = float.Parse(xLinearRaw, CultureInfo.InvariantCulture.NumberFormat),
                yPosLinear = float.Parse(yLinearRaw, CultureInfo.InvariantCulture.NumberFormat),
                RangeFL = float.Parse(RangeFLRaw, CultureInfo.InvariantCulture.NumberFormat),
                RangeFR = float.Parse(RangeFRRaw, CultureInfo.InvariantCulture.NumberFormat),
                RangeRR = float.Parse(RangeRRRaw, CultureInfo.InvariantCulture.NumberFormat),
                RangeRL = float.Parse(RangeRLRaw, CultureInfo.InvariantCulture.NumberFormat),
            });

            foreach (var s in Plots)
            {
                chart1.Series["Front"].Points.Clear();
                chart1.Series["Front"].Points.AddXY(s.xPosFront, s.yPosFront);
                //chart1.Series["Left"].Points.Clear();
                //chart1.Series["Left"].Points.AddXY(s.xPosLeft, s.yPosLeft);
                //chart1.Series["Back"].Points.Clear();
                //chart1.Series["Back"].Points.AddXY(s.xPosBack, s.yPosBack);
                //chart1.Series["Right"].Points.Clear();
                //chart1.Series["Right"].Points.AddXY(s.xPosRight, s.yPosRight);
                chart1.Series["Linear"].Points.Clear();
                chart1.Series["Linear"].Points.AddXY(s.xPosLinear, s.yPosLinear);
                label1.Text = "Position Linear: (" + s.xPosFront.ToString("0.00") + ", " + s.yPosFront.ToString("0.00") + ")";
            }
            Plots.Clear();



            label5.Text = "Distance (RL): " + yRaw + "cm";
            label4.Text = "Sampling: " + SamplingRaw;

            flrange.Text = "RangeFL: " + RangeFLRaw;
            frrange.Text = "RangeFR: " + RangeFRRaw;
            rrrange.Text = "RangeRR: " + RangeRRRaw;
            rlrange.Text = "RangeRL: " + RangeRLRaw;

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
            front.BackColor = Color.White;
            back.BackColor = Color.White;
            right.BackColor = Color.White;
            left.BackColor = Color.White;

        }

        private void log(object sender, EventArgs e)
        {
            if (IsLogging)
            {
                Logger.Add(new plot
                {
                    xPosRaw = float.Parse(xRaw, CultureInfo.InvariantCulture.NumberFormat),
                    yPosRaw = float.Parse(yRaw, CultureInfo.InvariantCulture.NumberFormat),
                    SamplingFreq = float.Parse(SamplingRaw, CultureInfo.InvariantCulture.NumberFormat),
                    Direction = float.Parse(Direction, CultureInfo.InvariantCulture.NumberFormat),
                    Forward = float.Parse(Forward, CultureInfo.InvariantCulture.NumberFormat),
                    xPosFront = float.Parse(xFrontRaw, CultureInfo.InvariantCulture.NumberFormat),
                    yPosFront = float.Parse(yFrontRaw, CultureInfo.InvariantCulture.NumberFormat),
                    xPosLeft = float.Parse(xLeftRaw, CultureInfo.InvariantCulture.NumberFormat),
                    yPosLeft = float.Parse(yLeftRaw, CultureInfo.InvariantCulture.NumberFormat),
                    xPosBack = float.Parse(xBackRaw, CultureInfo.InvariantCulture.NumberFormat),
                    yPosBack = float.Parse(yBackRaw, CultureInfo.InvariantCulture.NumberFormat),
                    xPosRight = float.Parse(xRightRaw, CultureInfo.InvariantCulture.NumberFormat),
                    yPosRight = float.Parse(yRightRaw, CultureInfo.InvariantCulture.NumberFormat),
                    xPosLinear = float.Parse(xLinearRaw, CultureInfo.InvariantCulture.NumberFormat),
                    yPosLinear = float.Parse(yLinearRaw, CultureInfo.InvariantCulture.NumberFormat),
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
                    tw.WriteLine(s.xPosRaw.ToString("0.00", new CultureInfo("en-US")) + "," + s.yPosRaw.ToString("0.00", new CultureInfo("en-US")) +
                        "," + s.xPosFilter.ToString("0.00", new CultureInfo("en-US")) + "," + s.yPosFilter.ToString("0.00", new CultureInfo("en-US")) +
                        "," + s.SamplingFreq.ToString("0.00", new CultureInfo("en-US")) +
                        "," + s.TxPower.ToString("0.00", new CultureInfo("en-US")) + "," + s.TxRxQ.ToString("0.00", new CultureInfo("en-US")));
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
            public double SamplingFreq { get; set; }
            public double Direction { get; set; }
            public double Forward { get; set; }
            public double xPosFront { get; set; }
            public double yPosFront { get; set; }
            public double xPosLeft { get; set; }
            public double yPosLeft { get; set; }
            public double xPosBack { get; set; }
            public double yPosBack { get; set; }
            public double xPosRight { get; set; }
            public double yPosRight { get; set; }
            public double xPosLinear { get; set; }
            public double yPosLinear { get; set; }
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
                    tw.WriteLine(s.xPosRaw.ToString("0.00", new CultureInfo("en-US")) + "," + s.yPosRaw.ToString("0.00", new CultureInfo("en-US")) +
                        "," + s.SamplingFreq.ToString("0.00", new CultureInfo("en-US")) + "," + s.Direction.ToString("0.00", new CultureInfo("en-US")) + 
                        "," + s.Forward.ToString("0.00", new CultureInfo("en-US")) + "," + s.RangeFL.ToString("0.00", new CultureInfo("en-US")) +
                        "," + s.RangeFR.ToString("0.00", new CultureInfo("en-US")) + "," + s.RangeRL.ToString("0.00", new CultureInfo("en-US")) +
                        "," + s.RangeRR.ToString("0.00", new CultureInfo("en-US")) + "," +  s.xPosFront.ToString("0.00", new CultureInfo("en-US")) + 
                        "," + s.yPosFront.ToString("0.00", new CultureInfo("en-US")) + "," + s.xPosLeft.ToString("0.00", new CultureInfo("en-US")) +
                        "," + s.yPosLeft.ToString("0.00", new CultureInfo("en-US")) + "," + s.xPosBack.ToString("0.00", new CultureInfo("en-US")) +
                        "," + s.yPosBack.ToString("0.00", new CultureInfo("en-US")) + "," + s.xPosRight.ToString("0.00", new CultureInfo("en-US")) +
                        "," + s.yPosRight.ToString("0.00", new CultureInfo("en-US")) + "," + s.xPosLinear.ToString("0.00", new CultureInfo("en-US")) +
                        "," + s.yPosLinear.ToString("0.00", new CultureInfo("en-US"))


                        );
                tw.Close();
                IsLogging = false;
            }
        }

    }

}
