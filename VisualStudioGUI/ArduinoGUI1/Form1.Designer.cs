namespace ArduinoGUI1
{
    partial class Form1
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            System.Windows.Forms.DataVisualization.Charting.ChartArea chartArea1 = new System.Windows.Forms.DataVisualization.Charting.ChartArea();
            System.Windows.Forms.DataVisualization.Charting.Legend legend1 = new System.Windows.Forms.DataVisualization.Charting.Legend();
            System.Windows.Forms.DataVisualization.Charting.Series series1 = new System.Windows.Forms.DataVisualization.Charting.Series();
            System.Windows.Forms.DataVisualization.Charting.Series series2 = new System.Windows.Forms.DataVisualization.Charting.Series();
            System.Windows.Forms.DataVisualization.Charting.Series series3 = new System.Windows.Forms.DataVisualization.Charting.Series();
            System.Windows.Forms.DataVisualization.Charting.Series series4 = new System.Windows.Forms.DataVisualization.Charting.Series();
            this.label1 = new System.Windows.Forms.Label();
            this.chart1 = new System.Windows.Forms.DataVisualization.Charting.Chart();
            this.cmbSerialPorts = new System.Windows.Forms.ComboBox();
            this.btnConnect = new System.Windows.Forms.Button();
            this.label2 = new System.Windows.Forms.Label();
            this.btnDisconnect = new System.Windows.Forms.Button();
            this.btnLogging = new System.Windows.Forms.Button();
            this.txFileName = new System.Windows.Forms.TextBox();
            this.label3 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.front = new System.Windows.Forms.PictureBox();
            this.back = new System.Windows.Forms.PictureBox();
            this.left = new System.Windows.Forms.PictureBox();
            this.right = new System.Windows.Forms.PictureBox();
            this.flrange = new System.Windows.Forms.Label();
            this.frrange = new System.Windows.Forms.Label();
            this.rrrange = new System.Windows.Forms.Label();
            this.rlrange = new System.Windows.Forms.Label();
            this.label6 = new System.Windows.Forms.Label();
            ((System.ComponentModel.ISupportInitialize)(this.chart1)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.front)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.back)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.left)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.right)).BeginInit();
            this.SuspendLayout();
            // 
            // label1
            // 
            this.label1.Font = new System.Drawing.Font("Microsoft Sans Serif", 26.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label1.Location = new System.Drawing.Point(777, 829);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(573, 42);
            this.label1.TabIndex = 0;
            this.label1.Text = "Position Linear: ";
            // 
            // chart1
            // 
            chartArea1.AlignmentStyle = System.Windows.Forms.DataVisualization.Charting.AreaAlignmentStyles.Cursor;
            chartArea1.AxisX.LabelStyle.Font = new System.Drawing.Font("Arial", 8F);
            chartArea1.AxisX.LineDashStyle = System.Windows.Forms.DataVisualization.Charting.ChartDashStyle.NotSet;
            chartArea1.AxisX.Maximum = 400D;
            chartArea1.AxisX.Minimum = -400D;
            chartArea1.AxisX.TitleFont = new System.Drawing.Font("Arial", 8F);
            chartArea1.AxisX2.LabelStyle.Font = new System.Drawing.Font("Arial", 8F);
            chartArea1.AxisX2.TitleFont = new System.Drawing.Font("Arial", 8F);
            chartArea1.AxisY.LabelStyle.Font = new System.Drawing.Font("Arial", 8F);
            chartArea1.AxisY.LineDashStyle = System.Windows.Forms.DataVisualization.Charting.ChartDashStyle.NotSet;
            chartArea1.AxisY.Maximum = 400D;
            chartArea1.AxisY.Minimum = -400D;
            chartArea1.AxisY.TitleFont = new System.Drawing.Font("Arial", 8F);
            chartArea1.AxisY2.LabelStyle.Font = new System.Drawing.Font("Arial", 8F);
            chartArea1.AxisY2.TitleFont = new System.Drawing.Font("Arial", 8F);
            chartArea1.Name = "Chart1";
            this.chart1.ChartAreas.Add(chartArea1);
            legend1.Font = new System.Drawing.Font("Arial", 8F);
            legend1.IsTextAutoFit = false;
            legend1.Name = "Legend1";
            legend1.TitleFont = new System.Drawing.Font("Arial", 8F, System.Drawing.FontStyle.Bold);
            this.chart1.Legends.Add(legend1);
            this.chart1.Location = new System.Drawing.Point(70, 79);
            this.chart1.Name = "chart1";
            series1.ChartArea = "Chart1";
            series1.Legend = "Legend1";
            series1.MarkerSize = 20;
            series1.MarkerStyle = System.Windows.Forms.DataVisualization.Charting.MarkerStyle.Square;
            series1.Name = "Robot";
            series2.ChartArea = "Chart1";
            series2.Legend = "Legend1";
            series2.MarkerSize = 10;
            series2.MarkerStyle = System.Windows.Forms.DataVisualization.Charting.MarkerStyle.Star10;
            series2.Name = "Front";
            series3.ChartArea = "Chart1";
            series3.Legend = "Legend1";
            series3.MarkerSize = 10;
            series3.Name = "Linear";
            series4.ChartArea = "Chart1";
            series4.Legend = "Legend1";
            series4.MarkerSize = 10;
            series4.Name = "Kalman";
            this.chart1.Series.Add(series1);
            this.chart1.Series.Add(series2);
            this.chart1.Series.Add(series3);
            this.chart1.Series.Add(series4);
            this.chart1.Size = new System.Drawing.Size(1019, 747);
            this.chart1.TabIndex = 1;
            this.chart1.Text = "chart1";
            // 
            // cmbSerialPorts
            // 
            this.cmbSerialPorts.FormattingEnabled = true;
            this.cmbSerialPorts.Location = new System.Drawing.Point(251, 19);
            this.cmbSerialPorts.Name = "cmbSerialPorts";
            this.cmbSerialPorts.Size = new System.Drawing.Size(121, 21);
            this.cmbSerialPorts.TabIndex = 2;
            // 
            // btnConnect
            // 
            this.btnConnect.Location = new System.Drawing.Point(387, 19);
            this.btnConnect.Name = "btnConnect";
            this.btnConnect.Size = new System.Drawing.Size(75, 20);
            this.btnConnect.TabIndex = 3;
            this.btnConnect.Text = "Connect";
            this.btnConnect.UseVisualStyleBackColor = true;
            this.btnConnect.Click += new System.EventHandler(this.btnConnect_Click);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(390, 44);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(73, 13);
            this.label2.TabIndex = 4;
            this.label2.Text = "Disconnected";
            // 
            // btnDisconnect
            // 
            this.btnDisconnect.Location = new System.Drawing.Point(467, 19);
            this.btnDisconnect.Margin = new System.Windows.Forms.Padding(2);
            this.btnDisconnect.Name = "btnDisconnect";
            this.btnDisconnect.Size = new System.Drawing.Size(74, 20);
            this.btnDisconnect.TabIndex = 5;
            this.btnDisconnect.Text = "Disconnect";
            this.btnDisconnect.UseVisualStyleBackColor = true;
            this.btnDisconnect.Click += new System.EventHandler(this.button2_Click);
            // 
            // btnLogging
            // 
            this.btnLogging.Location = new System.Drawing.Point(393, 62);
            this.btnLogging.Name = "btnLogging";
            this.btnLogging.Size = new System.Drawing.Size(148, 21);
            this.btnLogging.TabIndex = 6;
            this.btnLogging.Text = "Start Logging";
            this.btnLogging.UseVisualStyleBackColor = true;
            this.btnLogging.Click += new System.EventHandler(this.btnLogging_Click);
            // 
            // txFileName
            // 
            this.txFileName.Location = new System.Drawing.Point(251, 63);
            this.txFileName.Name = "txFileName";
            this.txFileName.Size = new System.Drawing.Size(121, 20);
            this.txFileName.TabIndex = 7;
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(193, 66);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(52, 13);
            this.label3.TabIndex = 8;
            this.label3.Text = "Filename:";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(67, 44);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(56, 13);
            this.label4.TabIndex = 10;
            this.label4.Text = "Sampling: ";
            // 
            // label5
            // 
            this.label5.Font = new System.Drawing.Font("Microsoft Sans Serif", 26.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label5.Location = new System.Drawing.Point(777, 871);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(653, 42);
            this.label5.TabIndex = 11;
            this.label5.Text = "Position Circles:";
            // 
            // front
            // 
            this.front.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.front.Location = new System.Drawing.Point(1196, 125);
            this.front.Name = "front";
            this.front.Size = new System.Drawing.Size(101, 89);
            this.front.TabIndex = 12;
            this.front.TabStop = false;
            // 
            // back
            // 
            this.back.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.back.Location = new System.Drawing.Point(1196, 305);
            this.back.Name = "back";
            this.back.Size = new System.Drawing.Size(101, 89);
            this.back.TabIndex = 13;
            this.back.TabStop = false;
            // 
            // left
            // 
            this.left.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.left.Location = new System.Drawing.Point(1095, 213);
            this.left.Name = "left";
            this.left.Size = new System.Drawing.Size(102, 93);
            this.left.TabIndex = 14;
            this.left.TabStop = false;
            // 
            // right
            // 
            this.right.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.right.Location = new System.Drawing.Point(1296, 213);
            this.right.Name = "right";
            this.right.Size = new System.Drawing.Size(102, 93);
            this.right.TabIndex = 15;
            this.right.TabStop = false;
            // 
            // flrange
            // 
            this.flrange.AutoSize = true;
            this.flrange.Font = new System.Drawing.Font("Microsoft Sans Serif", 15.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.flrange.Location = new System.Drawing.Point(1194, 487);
            this.flrange.Name = "flrange";
            this.flrange.Size = new System.Drawing.Size(112, 25);
            this.flrange.TabIndex = 16;
            this.flrange.Text = "RangeFL: ";
            // 
            // frrange
            // 
            this.frrange.AutoSize = true;
            this.frrange.Font = new System.Drawing.Font("Microsoft Sans Serif", 15.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.frrange.Location = new System.Drawing.Point(1194, 512);
            this.frrange.Name = "frrange";
            this.frrange.Size = new System.Drawing.Size(115, 25);
            this.frrange.TabIndex = 17;
            this.frrange.Text = "RangeFR: ";
            // 
            // rrrange
            // 
            this.rrrange.AutoSize = true;
            this.rrrange.Font = new System.Drawing.Font("Microsoft Sans Serif", 15.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.rrrange.Location = new System.Drawing.Point(1191, 537);
            this.rrrange.Name = "rrrange";
            this.rrrange.Size = new System.Drawing.Size(117, 25);
            this.rrrange.TabIndex = 18;
            this.rrrange.Text = "RangeRR: ";
            // 
            // rlrange
            // 
            this.rlrange.AutoSize = true;
            this.rlrange.Font = new System.Drawing.Font("Microsoft Sans Serif", 15.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.rlrange.Location = new System.Drawing.Point(1192, 562);
            this.rlrange.Name = "rlrange";
            this.rlrange.Size = new System.Drawing.Size(114, 25);
            this.rlrange.TabIndex = 19;
            this.rlrange.Text = "RangeRL: ";
            // 
            // label6
            // 
            this.label6.Font = new System.Drawing.Font("Microsoft Sans Serif", 26.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label6.Location = new System.Drawing.Point(777, 913);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(653, 42);
            this.label6.TabIndex = 11;
            this.label6.Text = "Position Kalman:";
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.SystemColors.Window;
            this.ClientSize = new System.Drawing.Size(1463, 1024);
            this.Controls.Add(this.rlrange);
            this.Controls.Add(this.rrrange);
            this.Controls.Add(this.frrange);
            this.Controls.Add(this.flrange);
            this.Controls.Add(this.right);
            this.Controls.Add(this.left);
            this.Controls.Add(this.back);
            this.Controls.Add(this.front);
            this.Controls.Add(this.label6);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.txFileName);
            this.Controls.Add(this.btnLogging);
            this.Controls.Add(this.btnDisconnect);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.btnConnect);
            this.Controls.Add(this.cmbSerialPorts);
            this.Controls.Add(this.chart1);
            this.Controls.Add(this.label1);
            this.Name = "Form1";
            this.Text = "Form1";
            this.Load += new System.EventHandler(this.Form1_Load);
            ((System.ComponentModel.ISupportInitialize)(this.chart1)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.front)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.back)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.left)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.right)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.DataVisualization.Charting.Chart chart1;
        private System.Windows.Forms.ComboBox cmbSerialPorts;
        private System.Windows.Forms.Button btnConnect;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Button btnDisconnect;
        private System.Windows.Forms.Button btnLogging;
        private System.Windows.Forms.TextBox txFileName;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.PictureBox front;
        private System.Windows.Forms.PictureBox back;
        private System.Windows.Forms.PictureBox left;
        private System.Windows.Forms.PictureBox right;
        private System.Windows.Forms.Label flrange;
        private System.Windows.Forms.Label frrange;
        private System.Windows.Forms.Label rrrange;
        private System.Windows.Forms.Label rlrange;
        private System.Windows.Forms.Label label6;
    }
}

