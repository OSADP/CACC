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
            int interval = 5 * simresol; // update interval. Global
            int vid = 0;
            int type = 0;
            int vlink = 0;
            int vLinkTotalLane = 0;
            int vlane = 0;
            object VTYPE = null;
            object VIDs = null;
            object VLinksLanes=null;
            object LinkIDs = null;
            object LinkTotalLanes = null;
            Dictionary<int, List<int>> VehCheckList = new Dictionary<int, List<int>>();
            Dictionary<int, List<int>> SwLinks = new Dictionary<int, List<int>>();
            List<int> SwitchVehs = new List<int>();

            //invoking the initiallizatio of the Vssim network based on the pararmeter defined above
            VissimTools.InitVissim(fn, fnini, SimTime, simresol, rseed);

            AdjustVolumeMulti_OnRampMerge(gpx, hovx, gpcaccx, hovcaccx);
            LinkIDs = VissimTools.GetLinkIDs();
            LinkTotalLanes = VissimTools.GetLinksTotalLanes();

            for (int i = 1; i <= simresol * SimTime - 1; i++)
            {
                VissimTools.vissim.Simulation.RunSingleStep();

                li++;

                if (li == interval)
                {
                    VIDs = VissimTools.GetAllVehiclesByID();
                    VTYPE = VissimTools.GetAllVehiclesByType();
                    VLinksLanes = VissimTools.GetAllVehiclesByLane();
                    int totNumVehs=((object[,])(VIDs)).Length/2;

                    object[,] updateVType=(object[,])(VTYPE);

                    for (int k = 0; k < totNumVehs; k++)
                    {
                        type = Convert.ToInt32(((object[,])(VTYPE))[k, 1]);
                        vid = Convert.ToInt32(((object[,])(VIDs))[k, 1]);
                        
                        string tmp = null;
                        tmp = Convert.ToString(((object[,])(VLinksLanes))[k, 1]);

                        vlink = int.Parse(tmp.Split("-".ToCharArray())[0]);
                        vlane = int.Parse(tmp.Split("-".ToCharArray())[1]);
                        //vLinkTotalLane = VissimTools.GetNumLanes(vlink);
                        int targetVType = 0;

                        int start=0;
                        int end=514;
                        int mid=(start+end)/2;
                        int temp=Convert.ToInt32(((object[,])(LinkIDs))[mid, 1]);
                        while(temp!=vlink)
                        {
                            if(temp>vlink)
                            {
                                end=mid-1;
                            }
                            else
                                start=mid+1;
                            mid=(start+end)/2;
                            temp = Convert.ToInt32(((object[,])(LinkIDs))[mid, 1]);
                        }
                        vLinkTotalLane = Convert.ToInt32(((object[,])(LinkTotalLanes))[mid, 1]);
                        
                        // incoming CACC lane changing control through veh type changes                        

                        if (type == 102 || type == 105)
                        {
                            if (vLinkTotalLane > 3)
                            {
                                if (vlane == vLinkTotalLane)
                                {
                                    if(type!=101)
                                        targetVType=101;
                                }
                                else if (vlane == vLinkTotalLane - 1)
                                {
                                    if(type!=104)
                                        targetVType = 104;
                                }
                                else
                                {
                                    VissimTools.SetVehDesLane(vid, vLinkTotalLane - 1);
                                    continue;
                                }
                            }
                        }
                        
                        if (type == 104)
                        {
                            //lane changing control for vehicle type 104

                            if (vlink == 1636 || vlink == 19)
                            {
                                vLinkTotalLane = vLinkTotalLane - 1;
                            }

                            if (vlane == vLinkTotalLane)
                            {
                                targetVType = 101;
                            }
                        }

                        if (type == 102 || type == 105 || type==101 || type==104)
                        {
                            //outgoing control for all the vehicle types
                            // CACC vehicles taking the off-ramp, set desired lane as 5 or 1 depending on left-exit or right-exit

                            if (!VehCheckList.ContainsKey(vlink))
                                VehCheckList.Add(vlink, new List<int>());

                            if (!VehCheckList[vlink].Contains(vid))
                            {
                                if (CheckRouteMulti(vlink, vid))
                                {                                    
                                    targetVType = 100;

                                    if (vlink == 1636 || vlink == 19)
                                    {
                                        VissimTools.SetVehDesLane(vid, 5);
                                    }
                                    else
                                    {
                                        VissimTools.SetVehDesLane(vid, 1);
                                    }
                                }
                                VehCheckList[vlink].Add(vid);
                            }                            
                        }
                        if(targetVType!=0)
                            updateVType[k, 1] = targetVType;
                    }
                    li = 0;
                    VissimTools.SetVTypeBulk(updateVType);
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

        private void AdjustVolumeMulti_OnRampMerge(double gpx, double hovx, double gpcaccx, double hovcaccx)
        {
            double vol1 = 0.0;
            double vol2 = 0.0;

            label36.Text = "Progress: Update Demand.... 0 %";

            for (int i = 9; i <= 23; i++) // 23
            {
                for (int j = 1; j <= 20; j++) //20
                {
                    vol1 = VissimTools.GetVol(i, "Volume(" + j + ")") * gpx;
                    vol2 = VissimTools.GetVol(i + 15, "Volume(" + j + ")") * hovx;

                    // update volume after CACC
                    VissimTools.SetVol(i, "Volume(" + j + ")", vol1 * (1.0 - gpcaccx));
                    VissimTools.SetVol(i + 15, "Volume(" + j + ")", vol2 * (1.0 - hovcaccx));
                    VissimTools.SetVol(i + 30, "Volume(" + j + ")", vol1 * gpcaccx + vol2 * hovcaccx);
                }

                label36.Text = "Progress: Update Demand.... " + Math.Round(Convert.ToDouble(100.0 * (i / 23)), 1) + " %";
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
            //List<int> MainVolInput = new List<int>();
            //List<int> OnRampVolInput = new List<int>();
            List<int> OutGoingLinks = new List<int>();
            List<int> InComingLinks = new List<int>();
            //List<int> MainVol = new List<int>();
            //List<int> OnRampVol = new List<int>();
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

            //MainVolInput = GetList(MainVolInput, maininput);
            //OnRampVolInput = GetList(OnRampVolInput, onrampinput);
            OutGoingLinks = GetList(OutGoingLinks, outgoinglinks); //
            InComingLinks = GetList(InComingLinks, incominglinks); //1513
            GetDestList(destlinks); // 1513,9:1526,1539|1555:1568,1557|1581:1590|1588:1596|1609:1618,1626|18:1667,1657|1661:1677|1636:1645|19:1604
            //MainVol = GetList(MainVol, Mainvol);
            //OnRampVol = GetList(OnRampVol, OnRVol);


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

                GPX = Math.Round(Convert.ToDouble(token[1]) / 100, 2);
                HOVX = Math.Round(Convert.ToDouble(token[2]) / 100, 2);
                GPCAX = Math.Round(Convert.ToDouble(token[3]) / 100, 2);
                HOVCAX = Math.Round(Convert.ToDouble(token[4]) / 100, 2);

                //double CACC_Main, double GP_Main, double CACC_Ramp, double GP_Ramp
                for (int i = 1; i <= NumRep; i++)
                {
                    RunCACCSimMulti(fn, fnini, sp, 10, rseed, OutGoingLinks, InComingLinks, GPX, HOVX, GPCAX, HOVCAX);
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

        private void textBox29_TextChanged(object sender, EventArgs e)
        {

        }

        private void textBox23_TextChanged(object sender, EventArgs e)
        {

        }

        private void textBox22_TextChanged(object sender, EventArgs e)
        {

        }

        private void textBox24_TextChanged(object sender, EventArgs e)
        {

        }

        private void textBox28_TextChanged(object sender, EventArgs e)
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
        public static object GetAllVehicles() { return vissim.Net.Vehicles; } //??? does it work??
        public static object GetAllVehiclesByID() { return VissimTools.vissim.Net.Vehicles.get_GetMultiAttValues("No"); } //OK
        public static object GetAllVehiclesByType() { return vissim.Net.Vehicles.get_GetMultiAttValues("VehType"); } // OK
        public static object GetAllVehiclesByLane() { return vissim.Net.Vehicles.get_GetMultiAttValues("Lane"); }
        public static object GetAllVehiclesByLink() { return vissim.Net.Vehicles.get_GetMultiAttValues("Link"); }
        public static object GetLinkIDs() { return vissim.Net.Links.get_GetMultiAttValues("No"); }
        public static object GetLinksTotalLanes() { return vissim.Net.Links.get_GetMultiAttValues("NUMLANES"); }
        public static object GetLinkVehiclesbyNumber(int lkn) { return vissim.Net.Links.get_ItemByKey(lkn).Vehs.get_GetMultiAttValues("No"); } // OK
        public static object GetLinkVehiclesByType(int lkn) { return vissim.Net.Links.get_ItemByKey(lkn).Vehs.get_GetMultiAttValues("VehType"); } // OK
        public static void SetVTypeBulk(object[,] updateVType) { vissim.Net.Vehicles.SetMultiAttValues("VehType", updateVType); }     

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
