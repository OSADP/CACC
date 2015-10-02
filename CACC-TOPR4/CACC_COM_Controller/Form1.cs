using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO;
using VISSIMLIB;

/*   VISSIM COM program to control CACC vehicles   
	updated 10-02-2015
	This code has some hard-coded lines that apply only to the I-66 WB network.
	The main task of this code is changing the vehicle type when needed, between 101, 102, 103, 104 and 105.
	Future developers can change this code to fit their network gemoetries, mainly changing the incoming/outgoing link IDs
*/
namespace CACCManGUI
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }

        private void textBox1_TextChanged(object sender, EventArgs e)
        {

        }

        private void button1_Click(object sender, EventArgs e)
        {
            List<int> MainVolInput = new List<int>();
            List<int> OnRampVolInput = new List<int>();
            List<int> OutGoingLinks = new List<int>();
            List<int> InComingLinks = new List<int>();
            List<int> MainVol = new List<int>();
            List<int> OnRampVol = new List<int>();
           

            StreamWriter sw = new StreamWriter(Directory.GetParent(textBox13.Text) + "\\CACCConf.dat");

            sw.WriteLine(Math.Round(Convert.ToDouble(textBox1.Text) / 3.0,1)); // ft/s -> m/s
            sw.WriteLine(Math.Round(Convert.ToDouble(textBox2.Text) / 3.0, 1)); // ft/s -> m/s
            sw.WriteLine(Math.Round(Convert.ToDouble(textBox3.Text) / 3.0, 1)); // ft/s -> m/s
            sw.WriteLine(Math.Round(Convert.ToDouble(textBox4.Text) / 3.0, 1));
            sw.WriteLine(Math.Round(Convert.ToDouble(textBox5.Text) * 1.604/3.6, 1)); // mi/h -> m/s
            sw.WriteLine(Math.Round(Convert.ToDouble(textBox6.Text) * 1.604/3.6, 1)); // mi/h -> m/s
            sw.WriteLine(textBox7.Text);
            sw.WriteLine(Math.Round(Convert.ToDouble(textBox8.Text) / 3.0, 1));
            sw.WriteLine(Math.Round(Convert.ToDouble(textBox9.Text) / 3.0, 1));
            sw.WriteLine(Math.Round(Convert.ToDouble(textBox10.Text) / 1.0, 1));
            sw.WriteLine(Math.Round(Convert.ToDouble(textBox11.Text) / 1.0, 1));
            sw.WriteLine(textBox47.Text); // Staging Link


            sw.Close();

            string fn = textBox12.Text;
            string fnini = textBox13.Text;
            int sp = int.Parse(textBox14.Text);
            int NumRep = int.Parse(textBox15.Text);
            int rseed = int.Parse(textBox16.Text);
            string maininput = textBox18.Text;
            string onrampinput = textBox19.Text;
            string outgoinglinks = textBox20.Text;
            string incominglinks = textBox21.Text;
            string Mainvol = textBox42.Text;
            string OnRVol = textBox43.Text;
            string MPCacc = textBox44.Text;
            string MPAcc = textBox45.Text;
            string MPHov = textBox46.Text;

            MainVolInput = GetList(MainVolInput, maininput);
            OnRampVolInput = GetList(OnRampVolInput, onrampinput);
            OutGoingLinks = GetList(OutGoingLinks, outgoinglinks); // 9,1513,1555,1581,1588,1597,1609,1636,1644,1661,19
            InComingLinks = GetList(InComingLinks, incominglinks); // 1515, 1513, 1529, 1555, 2008, 1565,1578, 1594,1600, 87,1640,  1644, 1661,1676, 1678,1679, 1687,20,10035,10036,19,1605,
            MainVol = GetList(MainVol, Mainvol);
            OnRampVol = GetList(OnRampVol, OnRVol);

            for (int i = 1; i <= NumRep; i++)
            {
                RunCACCSimSingle(fn, fnini, sp, 10, rseed, MainVolInput, OnRampVolInput, OutGoingLinks, InComingLinks, MainVol, OnRampVol, MPCacc, MPAcc, MPHov);
                rseed++;
            }
        }

        private static Dictionary<int, List<int>> DestLinks = new Dictionary<int, List<int>>();
        private static List<int> DestLinkList = new List<int>();

        private void label24_Click()
        {
        
        }

        private void textBox12_TextChanged(object sender, EventArgs e)
        {
        
        }

        private void RunCACCSimSingle(string fn, string fnini, int SimTime, int simresol, int rseed, List<int> MainVolInput, List<int> OnRampVolInput, List<int> OutGoingLinks, List<int> InComingLinks, List<int> MainVol, List<int> OnRampVol, string MPCacc, string MPAcc, string MPHov)
        {
            int outci = 0; //initialized the counter for traffic on on-ramp 
            int inci = 0; //initialized the time coutner for traffic on off-ramp
            int ininterval = 2 * simresol; // In the on-ramp (shorter length) collect data at twice of the simulation interval
            int outinterval = 4 * simresol; // In the off-ramp (longer length) collect data at four times of the simulation interval
            int vid = 0;
            int type = 0;
            object VTYPE = null;
            object VIDs = null;
            VissimTools.InitVissim(fn, fnini, SimTime, simresol, rseed);

            AdjustVolume(Convert.ToDouble(MPCacc));

            for (int i = 1; i <= simresol * SimTime; i++)
            {
                VissimTools.vissim.Simulation.RunSingleStep();

                inci++;
                outci++;

                if (inci == ininterval)
                {
                    foreach (int val in InComingLinks)
                    {

                        VIDs = VissimTools.GetLinkVehiclesbyNumber(val);
                        VTYPE = VissimTools.GetLinkVehiclesByType(val);

                        for (int k = 0; k < ((object[,])(VIDs)).Length / 2; k++)
                        {
                            type = Convert.ToInt32(((object[,])(VTYPE))[k, 1]);

                            if (type == 102)
                            {
                                vid = Convert.ToInt32(((object[,])(VIDs))[k, 1]);

                                if (VissimTools.Get_Lane(vid) == VissimTools.GetNumLanes(val) )
                                {
                                    if (val == 1581)
                                        Console.Write("!");
                                    VissimTools.Set_VehType(vid, 101);
                                }
                            }
                        }

                        inci = 0;
                    }
                }

                if (outci == outinterval)
                {
                    foreach (int val in OutGoingLinks)
                    {
                        VIDs = VissimTools.GetLinkVehiclesbyNumber(val);
                        VTYPE = VissimTools.GetLinkVehiclesByType(val);

                        for (int k = 0; k < ((object[,])(VIDs)).Length / 2; k++)
                        {
                            type = Convert.ToInt32(((object[,])(VTYPE))[k, 1]);

                            if (type == 101)
                            {
                                vid = Convert.ToInt32(((object[,])(VIDs))[k, 1]);

                                if (CheckRoute(vid))
                                {
                                    VissimTools.Set_VehType(vid, 100);
                                }
                            }
                        }

                        outci = 0;
                    }


                }

            }
        }

        private void RunCACCSimMultiOLD(string fn, string fnini, int SimTime, int simresol, int rseed, List<int> OutGoingLinks, List<int> InComingLinks, double gpx,double hovx,double gpcaccx,double hovcaccx)
        {
            int outci = 0; //initialized the counter for traffic on on-ramp 
            int inci = 0; //initialized the time coutner for traffic on off-ramp
            int swci = 0;
            int li = 0;            
            int ininterval = 2 * simresol; // In the on-ramp (shorter length) collect data at twice of the simulation interval
            int outinterval = 2 * simresol; // In the off-ramp (longer length) collect data at four times of the simulation interval
            int switchinterval = -2 * simresol; // in case of the Links 1600 and 1636 which have the middle-lane HOV exit, CACC needs to be temporarly switched to HOV to properly take the routes 
            int lowspdinterval = 2 * simresol;
            int vid = 0;
            int type = 0;
            int lkid = 0;
            object VTYPE = null;
            object VIDs = null;
            Dictionary<int, List<int>> VehCheckList = new Dictionary<int, List<int>>();
            Dictionary<int, List<int>> SwLinks = new Dictionary<int, List<int>>();
            List<int> SwitchVehs = new List<int>(); 

            //invoking the initiallizatio of the Vssim network based on the pararmeter defined above
            VissimTools.InitVissim(fn, fnini, SimTime, simresol, rseed);

            AdjustVolumeMulti(gpx,hovx,gpcaccx,hovcaccx);

            // Hard Coded for the TOPR 4 Project
            SwLinks.Add(1600, new List<int>()); SwLinks[1600].Add(1604);
            SwLinks.Add(1636, new List<int>()); SwLinks[1636].Add(1645); 


            double lowspd = 50.0;
            int lk = 0;
            int tglane = 0;

            for (int i = 1; i <= simresol * SimTime-1; i++)
            {
                VissimTools.vissim.Simulation.RunSingleStep();


                inci++;
                outci++;
                swci++;
                li++;

                if (li == lowspdinterval)
                {

                    VIDs = VissimTools.GetAllVehicles();
                    VTYPE = VissimTools.GetAllVehiclesByType();

                    for (int k = 0; k < ((object[,])(VIDs)).Length / 2; k++)
                    {
                        type = Convert.ToInt32(((object[,])(VTYPE))[k, 1]);

                        if (type == 101)
                        {
                            vid = Convert.ToInt32(((object[,])(VIDs))[k, 1]);
                            lk = VissimTools.Get_VehLink(vid);

                            if (VissimTools.Get_VehSpd(vid) <= lowspd && (VissimTools.Get_Lane(vid) == VissimTools.GetNumLanes(lk)))
                            {
                                VissimTools.Set_VehType(vid, 103);                            
                            }
                        }

                        if (type == 103)
                        {
                            vid = Convert.ToInt32(((object[,])(VIDs))[k, 1]);
                            lk = VissimTools.Get_VehLink(vid);

                            if (VissimTools.Get_VehSpd(vid) > 1.2*lowspd && VissimTools.Get_Lane(vid) == VissimTools.GetNumLanes(lk))
                            {
                                VissimTools.Set_VehType(vid, 101);
                            }
                        }
                    }

                    li = 0;
 
                
                }

                if (inci == ininterval)
                {
                    foreach (int val in InComingLinks)
                    {

                        
                        VIDs = VissimTools.GetLinkVehiclesbyNumber(val);
                        VTYPE = VissimTools.GetLinkVehiclesByType(val);


                        for (int k = 0; k < ((object[,])(VIDs)).Length / 2; k++)
                        {
                            type = Convert.ToInt32(((object[,])(VTYPE))[k, 1]);

                            if (type == 102)
                            {
                                vid = Convert.ToInt32(((object[,])(VIDs))[k, 1]);

                                tglane = VissimTools.GetNumLanes(val);

                                if (VissimTools.Get_Lane(vid) == tglane)
                                {
                                    VissimTools.Set_VehType(vid, 101);
                                }
                                else
                                    VissimTools.SetVehDesLane(vid, tglane);

                            }
                        }
                        inci = 0;
                    }
                }



                if (outci == outinterval)
                {
                    foreach (int val in OutGoingLinks)
                    {
                        VIDs = VissimTools.GetLinkVehiclesbyNumber(val);
                        VTYPE = VissimTools.GetLinkVehiclesByType(val);

                        for (int k = 0; k < ((object[,])(VIDs)).Length / 2; k++)
                        {
                            type = Convert.ToInt32(((object[,])(VTYPE))[k, 1]);

                            if (type == 101)
                            {
                                if (!VehCheckList.ContainsKey(val))
                                    VehCheckList.Add(val, new List<int>());

                                vid = Convert.ToInt32(((object[,])(VIDs))[k, 1]);

                                if (!VehCheckList[val].Contains(vid))
                                {
                                    if (CheckRouteMulti(val, vid))
                                    {
                                        if (val == 1636 || val == 19)
                                        {
                                            VissimTools.Set_VehType(vid, 102);
                                            VissimTools.SetVehDesLane(vid, 5);
                                        }
                                        else
                                        {
                                            VissimTools.Set_VehType(vid, 100);
                                            VissimTools.SetVehDesLane(vid, 1);
                                        }
                                    }

                                    VehCheckList[val].Add(vid);
                                }
                            }
                        }

                        outci = 0;
                    }
                }

                if (swci == switchinterval)
                {
                    foreach (int val in SwLinks.Keys)
                    {
                        VIDs = VissimTools.GetLinkVehiclesbyNumber(val);
                        VTYPE = VissimTools.GetLinkVehiclesByType(val);

                        for (int k = 0; k < ((object[,])(VIDs)).Length / 2; k++)
                        {
                            type = Convert.ToInt32(((object[,])(VTYPE))[k, 1]);
                            vid = Convert.ToInt32(((object[,])(VIDs))[k, 1]);

                            if (vid == 270)
                                Console.Beep(); 

                            if (!SwitchVehs.Contains(vid))
                            {
                                if (type == 101)
                                {
                                    if (VissimTools.Get_Lane(vid) == VissimTools.GetNumLanes(val))
                                    {
                                        if (!SwLinks[val].Contains(VissimTools.GetDestLink(vid)))
                                        {
                                            VissimTools.Set_VehType(vid, 700);
                                            SwitchVehs.Add(vid);
                                        }
                                        else
                                        {
                                            VissimTools.Set_VehType(vid, 700);                                        
                                        }
                                    }
                                }
                            }
                            else
                            {
                                if (VissimTools.Get_Lane(vid) == (VissimTools.GetNumLanes(val) - 1))
                                {
                                    VissimTools.Set_VehType(vid, 101);
                                    SwitchVehs.Remove(vid);
                                }

                            }
                        }
                    }
                    swci = 0;

                }

            }
        }

        private void RunCACCSimMulti(string fn, string fnini, int SimTime, int simresol, int rseed, List<int> OutGoingLinks, List<int> InComingLinks, double gpx, double hovx, double gpcaccx, double hovcaccx)
        {
            /*
             * Vehcle Type Definition
             * 101 : CACC vehicle in CACC Mode
             * 102 : pre-CACC vehicles from On-Ramp. They are controlled by COM but Will turn to 104 once they get into the second left-most lane to look for a chace to get into the left-most lane  
             * 103 : off-CACC vehicles due to low speed. Will return to 101 once the speed is recovered
             * 104 : (Newly Added) pre-CACC vehicles looking for a chance to join CACC platoon. Controlled by API. Turn to 101 once joining to the platoon
             * 105 : (Newly Added) spin-off of 102. It is only for CACC vehicles depart the network at the beginning. Will turn to 101 once they getting into the left-most lane of mainline
             */



            int li = 0; 
            int interval = 2 * simresol; // update interval. Global
            int vid = 0;
            int type = 0;
            int lkid = 0;
            object VTYPE = null;
            object VIDs = null;
            Dictionary<int, List<int>> VehCheckList = new Dictionary<int, List<int>>();
            Dictionary<int, List<int>> SwLinks = new Dictionary<int, List<int>>();
            List<int> SwitchVehs = new List<int>();

            //invoking the initiallizatio of the Vssim network based on the pararmeter defined above
            VissimTools.InitVissim(fn, fnini, SimTime, simresol, rseed);

            AdjustVolumeMulti(gpx, hovx, gpcaccx, hovcaccx);
           

            double lowspd = 20.0; // kph
            int lk = 0;
            int tglane = 0;

            for (int i = 1; i <= simresol * SimTime - 1; i++)
            {
                VissimTools.vissim.Simulation.RunSingleStep();

                li++;

                if (li == interval)
                {

                    VIDs = VissimTools.GetAllVehicles();
                    VTYPE = VissimTools.GetAllVehiclesByType();

                    for (int k = 0; k < ((object[,])(VIDs)).Length / 2; k++)
                    {
                        type = Convert.ToInt32(((object[,])(VTYPE))[k, 1]);

                        // incoming CACC Control
                        if (type == 102 || type == 105)
                        {
                            vid = Convert.ToInt32(((object[,])(VIDs))[k, 1]);
                            lk = VissimTools.Get_VehLink(vid);

                            tglane = VissimTools.GetNumLanes(lk);


                            // for those who are on the mainline
                            if (tglane > 3)
                            {

                                if (VissimTools.Get_Lane(vid) == tglane)
                                {
                                    VissimTools.Set_VehType(vid, 101);
                                }

                                if (type == 102 && VissimTools.Get_Lane(vid) == tglane - 1)
                                {
                                    VissimTools.Set_VehType(vid, 104);

                                }
                                else
                                {
                                    VissimTools.SetVehDesLane(vid, tglane - 1);
                                    continue;
                                }
                            }

                            // To control potential CACC vehicles (102) outgoing from the previous ramp to the next ramp.  

                            if (type == 102)
                            {
                                if (!VehCheckList.ContainsKey(lk))
                                    VehCheckList.Add(lk, new List<int>());

                                if (!VehCheckList[lk].Contains(vid))
                                {
                                    if (CheckRouteMulti(lk, vid))
                                    {
                                        VissimTools.Set_VehType(vid, 100);
                                        if (lk == 1636 || lk == 19)
                                        {
                                            
                                            VissimTools.SetVehDesLane(vid, 5);
                                        }
                                        else
                                        {
                                            VissimTools.SetVehDesLane(vid, 1);
                                        }
                                    }

                                    VehCheckList[lk].Add(vid);
                                }
                            
                            }

                        }

                        else if (type == 104)
                        {
                            vid = Convert.ToInt32(((object[,])(VIDs))[k, 1]);
                            lk = VissimTools.Get_VehLink(vid);
                            tglane = VissimTools.GetNumLanes(lk);

                            if (lk == 1636 || lk == 19)
                            {
                                tglane = tglane - 1;
                            }

                            if (VissimTools.Get_Lane(vid) == tglane)
                            {
                                VissimTools.Set_VehType(vid, 101);
                            }


                            // outgoing CACC control for vtype 104

                            if (!VehCheckList.ContainsKey(lk))
                                VehCheckList.Add(lk, new List<int>());

                            if (!VehCheckList[lk].Contains(vid))
                            {
                                if (CheckRouteMulti(lk, vid))
                                {
                                    VissimTools.Set_VehType(vid, 100);

                                    if (lk == 1636 || lk == 19)
                                    {
                                        //VissimTools.Set_VehType(vid, 102);
                                        VissimTools.SetVehDesLane(vid, 5);
                                    }
                                    else
                                    {
                                        //VissimTools.Set_VehType(vid, 100);
                                        VissimTools.SetVehDesLane(vid, 1);
                                    }
                                }

                                VehCheckList[lk].Add(vid);
                            }
                        }

                        else if (type == 101)
                        {

                            vid = Convert.ToInt32(((object[,])(VIDs))[k, 1]);
                            lk = VissimTools.Get_VehLink(vid);


                            //low speed CACC control
                            if (VissimTools.Get_VehSpd(vid) <= lowspd && (VissimTools.Get_Lane(vid) == VissimTools.GetNumLanes(lk)))
                            {
                                
                                VissimTools.Set_VehType(vid, 103);
                                continue;
                            }


                            // outgoing CACC control

                            if (!VehCheckList.ContainsKey(lk))
                                VehCheckList.Add(lk, new List<int>());

                            if (!VehCheckList[lk].Contains(vid))
                            {
                                if (CheckRouteMulti(lk, vid))
                                {
                                    VissimTools.Set_VehType(vid, 100);

                                    if (lk == 1636 || lk == 19)
                                    {
                                        //VissimTools.Set_VehType(vid, 102);
                                        VissimTools.SetVehDesLane(vid, 5);
                                    }
                                    else
                                    {
                                        //VissimTools.Set_VehType(vid, 100);
                                        VissimTools.SetVehDesLane(vid, 1);
                                    }
                                }

                                VehCheckList[lk].Add(vid);
                            }

                        }

                        //low speed CACC control
                        else if (type == 103)
                        {
                            vid = Convert.ToInt32(((object[,])(VIDs))[k, 1]);
                            lk = VissimTools.Get_VehLink(vid);

                            if (VissimTools.Get_VehSpd(vid) > 1.2 * lowspd)
                            {
                                tglane = VissimTools.GetNumLanes(lk);

                                if (VissimTools.Get_Lane(vid) == tglane)
                                {
                                    VissimTools.Set_VehType(vid, 101);
                                }
                                else
                                {
                                    // Modified to change vtype 103 to 104
                                    if (lk == 1636 || lk == 19)
                                    {
                                        if (VissimTools.Get_Lane(vid) == tglane-2)
                                            VissimTools.Set_VehType(vid, 104);
                                        else
                                            VissimTools.SetVehDesLane(vid, tglane - 2);                                            
                                    }
                                    else
                                    {
                                        if (VissimTools.Get_Lane(vid) == tglane - 1)
                                            VissimTools.Set_VehType(vid, 104);
                                        else
                                            VissimTools.SetVehDesLane(vid, tglane - 1);    
                                    }
                                }
                            }
                        }
                    }
                    li = 0;
                }
                    
            }
        }

        private void AdjustVolume(double p)
        {
            double vol1 = 0.0;
            double vol2 = 0.0;

            for (int i=9;i<=23;i++)
            {
                for (int j = 1; j <= 20; j++)
                {
                    vol1 = VissimTools.GetVol(i, "Volume(" + j + ")");
                    vol2 = VissimTools.GetVol(i+15, "Volume(" + j + ")");
                    VissimTools.SetVol(i, "Volume(" + j + ")", vol1 * (1.0 - p));
                    VissimTools.SetVol(i+15, "Volume(" + j + ")", vol2 * (1.0 - p));
                    vol1 = vol1 * p;
                    vol2 = vol2 * p;
                    VissimTools.SetVol(i + 30, "Volume(" + j + ")", vol1+vol2);
                }
            }
        }


        private void AdjustVolumeMulti(double gpx, double hovx, double gpcaccx, double hovcaccx)
        {
            double vol1 = 0.0;
            double vol2 = 0.0;

            label36.Text = "Progress: Update Demand.... 0 %";

            for (int i = 9; i <= 23; i++) // 23
            {
                for (int j = 1; j <= 20; j++) //20
                {
                    // current volume
                    vol1 = VissimTools.GetVol(i, "Volume(" + j + ")");
                    vol2 = VissimTools.GetVol(i + 15, "Volume(" + j + ")");

                    // udpate volume before CACC
                    VissimTools.SetVol(i, "Volume(" + j + ")", vol1 *gpx);
                    VissimTools.SetVol(i + 15, "Volume(" + j + ")", vol2*hovx);
                    vol1 = VissimTools.GetVol(i, "Volume(" + j + ")");
                    vol2 = VissimTools.GetVol(i + 15, "Volume(" + j + ")");

                    // update volume after CACC
                    VissimTools.SetVol(i, "Volume(" + j + ")", vol1 * (1.0-gpcaccx));
                    VissimTools.SetVol(i + 15, "Volume(" + j + ")", vol2 *(1.0-hovcaccx));
                    vol1 = vol1 * gpcaccx;
                    vol2 = vol2 * hovcaccx;
                    VissimTools.SetVol(i + 30, "Volume(" + j + ")", vol1 + vol2);
                }

                label36.Text = "Progress: Update Demand.... "+Math.Round(Convert.ToDouble(100.0*(i/23)),1)+" %";
                label36.Update();
            }
            label36.Text = "Progress: Simulation In Progress";
            label36.Update();
        }


        private bool CheckRoute(int vid)
        {
            if (DestLinkList.Contains(VissimTools.GetDestLink(vid)))
                return true;
            else
                return false;
        }

        private bool CheckRouteMulti(int lkid,int vid)
        {
            if (DestLinks.ContainsKey(lkid))
            {
                if (DestLinks[lkid].Contains(VissimTools.GetDestLink(vid)))
                   return true;
                else
                    return false;
            }
            else
                return false;
        }

        private void UpdateVolume(List<int> VolInput, List<int> Vol)
        {
            int p = 0;


            foreach (int val in VolInput)
            {
                VissimTools.SetVol(val, "Volume(1)", Vol[p]); //vehicle input point [VolInput], volume of 1st interval 
                //VissimTools.SetVol(val, "Volume(2)", Vol); //vehicle input point [VolInput], volume of 2nd interval 
                p++;
            }
        }

        private static List<int> GetList(List<int> List_, string str)
        {
            string[] tmp = null;

            tmp = str.Split(",".ToCharArray());

            for (int i = 0; i < tmp.Length; i++)
            {
                if (tmp[i].Length>0)
                    List_.Add(int.Parse(tmp[i]));
            }
            return List_;
        }


        private void button2_Click(object sender, EventArgs e)
        {
            Application.Exit();
        }

        private void button3_Click(object sender, EventArgs e)
        {
            openFileDialog1.ShowDialog();
            string str1 = openFileDialog1.FileName;
            textBox12.Text = str1;
        }

        private void button4_Click(object sender, EventArgs e)
        {
            openFileDialog1.ShowDialog();
            string str1 = openFileDialog1.FileName;
            textBox13.Text = str1;
        }

        private void Form1_Load(object sender, EventArgs e)
        {

        }

        private void button6_Click(object sender, EventArgs e)
        {
            List<int> OutGoingLinks = new List<int>();
            List<int> InComingLinks = new List<int>();
            string line = null;
            string[] token = null;


            StreamWriter sw;
            StreamReader sr = new StreamReader(textBox17.Text);

            
            string fn = textBox30.Text;
            string fnini = textBox29.Text;
            int sp = int.Parse(textBox28.Text);
            int NumRep = int.Parse(textBox27.Text);
            int rseed = int.Parse(textBox26.Text);

            string outgoinglinks = textBox23.Text;
            string incominglinks = textBox22.Text;
            string destlinks = textBox24.Text;

            double GPX = 0.0;
            double HOVX = 0.0;
            double GPCAX = 0.0;
            double HOVCAX = 0.0; 

            OutGoingLinks = GetList(OutGoingLinks, outgoinglinks); // 9,1513,1555,1581,1588,1597,1609,1636,1644,1661,19
            InComingLinks = GetList(InComingLinks, incominglinks); //1515, 1513, 1529, 1555, 2008, 1565,1578, 1594,1600, 87,1640,  1644, 1661,1676, 1678,1679, 1687,20,10035,10036,19,1605,
            GetDestList(destlinks); // 1513,9:1526,1539|1555:1568,1557|1581:1590|1588:1596|1609:1618,1626|18:1667,1657|1661:1677|1636:1645|19:1604


            while ((line = sr.ReadLine()) != null)
            {
                if (line.Contains("#"))
                    continue;
                token = line.Split(",".ToCharArray());

                sw = new StreamWriter(Directory.GetParent(textBox30.Text) + "\\CACCConf101.dat");
                sw.WriteLine(int.Parse(token[6])); // Max Platoon Size
                sw.WriteLine(Convert.ToDouble(token[7])); // CACC headway 
                sw.WriteLine(Convert.ToDouble(token[8])); // NonCACC headway 
                sw.Close();

                sw = new StreamWriter(Directory.GetParent(textBox30.Text) + "\\CACCConf104.dat");
                sw.WriteLine(Convert.ToDouble(token[9])); // Leading Critical Gap (second) 
                sw.WriteLine(Convert.ToDouble(token[10])); // Lagging Critical Gap (second) 
                sw.WriteLine(Math.Round(1.64*Convert.ToDouble(token[11])/3.6, 1)); // Leading Max Speed Differential (m/s)
                sw.WriteLine(Math.Round(1.64*Convert.ToDouble(token[12])/3.6, 1)); // Lagging Max Speed Differential (m/s)
                sw.WriteLine(int.Parse(token[5])); // toogle
                sw.Close();

                GPX = Math.Round(Convert.ToDouble(token[1])/100,2);
                HOVX = Math.Round(Convert.ToDouble(token[2]) / 100, 2);
                GPCAX = Math.Round(Convert.ToDouble(token[3]) / 100, 2);
                HOVCAX = Math.Round(Convert.ToDouble(token[4]) / 100, 2);


                for (int i = 1; i <= NumRep; i++)
                {
                    RunCACCSimMulti(fn, fnini, sp, 10, rseed, OutGoingLinks, InComingLinks, GPX,HOVX,GPCAX,HOVCAX);
                    rseed++;
                }
            }
        }

        private void GetDestList(string str)
        {
            string[] tmp1 = null; // for |
            string[] tmp2 = null; // for :
            string[] tmp3 = null; // for , left side
            string[] tmp4 = null; // for , right side

            tmp1 = str.Split("|".ToCharArray());

            for (int i = 0; i < tmp1.Length; i++)
            {
                if (tmp1[i].Length > 0)
                {
                    tmp2 = tmp1[i].Split(":".ToCharArray());
                    tmp3 = tmp2[0].Split(",".ToCharArray());
                    tmp4 = tmp2[1].Split(",".ToCharArray());

                    for (int j = 0; j < tmp3.Length; j++)
                    {
                        DestLinks.Add(int.Parse(tmp3[j]), new List<int>());
                        for (int k = 0; k < tmp4.Length; k++)
                        {
                            DestLinks[int.Parse(tmp3[j])].Add(int.Parse(tmp4[k]));                        
                        }
                    
                    }

                }
            }

        }

        private void tabPage1_Click(object sender, EventArgs e)
        {

        }

        private void button9_Click(object sender, EventArgs e)
        {
            Application.Exit();
        }

        private void textBox12_TextChanged()
        {

        }

        private void textBox30_TextChanged(object sender, EventArgs e)
        {

        }

        private void button8_Click(object sender, EventArgs e)
        {
            openFileDialog1.ShowDialog();
            string str1 = openFileDialog1.FileName;
            textBox30.Text = str1;
        }

        private void button7_Click(object sender, EventArgs e)
        {
            openFileDialog1.ShowDialog();
            string str1 = openFileDialog1.FileName;
            textBox29.Text = str1;
        }

        private void button5_Click(object sender, EventArgs e)
        {
            openFileDialog1.ShowDialog();
            string str1 = openFileDialog1.FileName;
            textBox17.Text = str1;
        }

        private void label36_Click(object sender, EventArgs e)
        {

        }
    }


    class VissimTools
    {
        static public Vissim vissim = new Vissim();

        static public void InitVissim(string fn, string fn_ini, int sp, int res, int rseed)
        {
            vissim.LoadNet(fn, false);
            vissim.LoadLayout(fn_ini);
            vissim.Simulation.set_AttValue("SimPeriod", sp);
            vissim.Simulation.set_AttValue("SimRes", res);
            vissim.Simulation.set_AttValue("RandSeed", rseed);

        }

        static public void EndVissim()
        {
            vissim.Exit();
        }


        //static public void Set_InputVol(int id, int vol) { vissim.Net.VehicleInputs.get_ItemByKey(id).set_AttValue("Volume", vol); } // not ok
        public static object GetLinkVehiclesbyNumber(int lkn) { return vissim.Net.Links.get_ItemByKey(lkn).Vehs.get_GetMultiAttValues("No"); } // OK
        public static object GetAllVehiclesByType() { return vissim.Net.Vehicles.get_GetMultiAttValues("VehType"); } // OK
        public static object GetLinkVehiclesByType(int lkn) { return vissim.Net.Links.get_ItemByKey(lkn).Vehs.get_GetMultiAttValues("VehType"); } // OK

        
        public static object GetAllVehicles() { return VissimTools.vissim.Net.Vehicles.get_GetMultiAttValues("No"); } //OK
        static public int Get_VehType(int vid) { return int.Parse(vissim.Net.Vehicles.get_ItemByKey(vid).get_AttValue("VehType")); } //OK
        static public double Get_VehSpd(int vid) {return Convert.ToDouble(vissim.Net.Vehicles.get_ItemByKey(vid).get_AttValue("Speed"));}
        static public int Get_VehLink(int vid)
        {
            string tmp = null;
            int lk_ = 0;
            tmp = vissim.Net.Vehicles.get_ItemByKey(vid).get_AttValue("Lane");
            lk_ = int.Parse(tmp.Split("-".ToCharArray())[0]);
            return lk_;
        }

        static public int Get_Lane(int vid)
        {
            string tmp = null;
            int lan_ = 0;
            tmp = vissim.Net.Vehicles.get_ItemByKey(vid).get_AttValue("Lane");
            lan_ = int.Parse(tmp.Split("-".ToCharArray())[1]);
            return lan_;
        } //OK
        static public void Set_VehType(int vid, int type) { vissim.Net.Vehicles.get_ItemByKey(vid).set_AttValue("VehType", type); }
        static public void SetVehDesLane(int vid, int lane) { vissim.Net.Vehicles.get_ItemByKey(vid).set_AttValue("DesLane", lane); }
        static public int GetRoute(int vid) { return (int)vissim.Net.Vehicles.get_ItemByKey(vid).get_AttValue("RouteNo"); }
        static public void SetVol(int id, string volbin, double vol) { vissim.Net.VehicleInputs.get_ItemByKey(id).set_AttValue(volbin, vol); }
        static public double GetVol(int id, string volbin) { return Convert.ToDouble(vissim.Net.VehicleInputs.get_ItemByKey(id).get_AttValue(volbin)); }
        static public void SetRelFlow(int id, string attr, double fr) { vissim.Net.VehicleCompositions.get_ItemByKey(id).set_AttValue(attr, fr); }
        static public int GetDestLink(int vid)
        {
            int RouteNo;
            int RouteDec;
            int DestLink;
            
            RouteNo = (int)vissim.Net.Vehicles.get_ItemByKey(vid).get_AttValue("RouteNo");
            RouteDec = (int)vissim.Net.Vehicles.get_ItemByKey(vid).get_AttValue("RoutDecNo");
            DestLink = Convert.ToInt32(vissim.Net.VehicleRoutingDecisionsStatic.get_ItemByKey(RouteDec).VehRoutSta.get_ItemByKey(RouteNo).get_AttValue("DestLink"));
            
            return DestLink;
        
        }
        static public int GetNumLanes(int lkid) { return (int)vissim.Net.Links.get_ItemByKey(lkid).get_AttValue("NumLanes"); }


    }
}
