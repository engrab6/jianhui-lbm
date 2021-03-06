#include<iostream>
#include<cmath>
#include<cstdlib>
#include<iomanip>
#include<fstream>
#include<sstream>
#include<string.h>
#include<math.h> 
# include "mpi.h"

//======PARMETIS===============
#include<parmetis.h>  
//mpic++ SINGLE_PHASE_MPI_Software_Spars_3DPartition.cpp -lparmetis -lmetis -o paratest
//mpic++ Multi_Component_CG_MPI_SPARSE_Software_3DPartition.cpp ./../parmetis-4.0.2/build/Linux-x86_64/libparmetis/libparmetis.a ./../parmetis-4.0.2/build/Linux-x86_64/libmetis/libmetis.a -o paratest
//=============================

#define MAXN 100
#define eps 1e-12
#define zero(x) (fabs(x)<eps)

struct mat{
    int n,m;
    double data[MAXN][MAXN];
};



//D3Q19 STANDARD LATTICE MRT LATTICE BOLTZMANN METHOD
//UNIFORM MESH, LATTICE VELOCITY 1


using namespace std;        
const int Q=19;          

int preci=2;
double u_max,u_ave,pre_u_ave,gx,gy,gz,porosity;

//----------
double s_e;
double s_eps;
double s_q;
//----------

double s_v;
double q_p;

int cl,cr;

int EI;


int Count,NX,NY,NZ;

int mirX=0;
int mirY=0;
int mirZ=0;
int mir=0;





double M[19][19]=
{{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
{-1,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1},
{1,-2,-2,-2,-2,-2,-2,1,1,1,1,1,1,1,1,1,1,1,1},
{0,1,-1,0,0,0,0,1,-1,1,-1,1,-1,1,-1,0,0,0,0},
{0,-2,2,0,0,0,0,1,-1,1,-1,1,-1,1,-1,0,0,0,0},
{0,0,0,1,-1,0,0,1,-1,-1,1,0,0,0,0,1,-1,1,-1},
{0,0,0,-2,2,0,0,1,-1,-1,1,0,0,0,0,1,-1,1,-1},
{0,0,0,0,0,1,-1,0,0,0,0,1,-1,-1,1,1,-1,-1,1},
{0,0,0,0,0,-2,2,0,0,0,0,1,-1,-1,1,1,-1,-1,1},
{0,2,2,-1,-1,-1,-1,1,1,1,1,1,1,1,1,-2,-2,-2,-2},
{0,-2,-2,1,1,1,1,1,1,1,1,1,1,1,1,-2,-2,-2,-2},
{0,0,0,1,1,-1,-1,1,1,1,1,-1,-1,-1,-1,0,0,0,0},
{0,0,0,-1,-1,1,1,1,1,1,1,-1,-1,-1,-1,0,0,0,0},
{0,0,0,0,0,0,0,1,1,-1,-1,0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,-1,-1},
{0,0,0,0,0,0,0,0,0,0,0,1,1,-1,-1,0,0,0,0},
{0,0,0,0,0,0,0,1,-1,1,-1,-1,1,-1,1,0,0,0,0},
{0,0,0,0,0,0,0,-1,1,1,-1,0,0,0,0,1,-1,1,-1},
{0,0,0,0,0,0,0,0,0,0,0,1,-1,-1,1,-1,1,1,-1}};


double MI[19][19];

double M_c[19];






double m[19];
double meq[19];





double uMax,c,Re,dx,dy,Lx,Ly,dt,rho0,P0,tau_f,niu,error,SFx,SFy,reso;

void Read_Rock(int***,double***, double*,char[128],char[128]);

void tests();

void init_Sparse_read_rock_parallel(int*, int*);

void init(double*, double**, double**,double*, double*,double*, double*, double*,int*);

void Partition_Solid(int***);

void periodic_streaming(double** ,double** ,int* ,int***,int*, int*,double*, double**);

void standard_bounceback_boundary(int,double**);

void collision(double*,double** ,double** ,double**, double*, double*, double*, double* ,double* , int* ,int***,int* ,int*);

void comput_macro_variables( double* ,double**,double** ,double** ,double** ,double*, double*, double*, double*, double* ,int* );

double Error(double** ,double** ,double*, double*);

void boundary_velocity(int,double,int , double ,int ,double ,int ,double ,int ,double ,int ,double ,double** ,double**,double* ,double** ,int*** );

void boundary_pressure(int ,double ,int , double ,int ,double ,int ,double ,int ,double ,int ,double ,double** ,double**,double** ,double* ,int*** );

void output_velocity(int ,double* ,double** ,int ,int ,int ,int ,int*** );

void output_density(int ,double* ,int ,int ,int ,int ,int*** );	

void Geometry(int*** );	

void Geometry_Par(int*** );

void output_psi(int ,double* ,int ,int ,int ,int ,int*** );

double Comput_Perm(double* ,double** ,double* ,double* ,int,int* );

double Comput_Saturation(double* ,int***,int*);

//double Comput_Saturation_disp(double* ,int***,int*);

double S[19];

void Comput_MI(double[19][19], double[19][19]);

int inverse(mat &a);

double feq(int,double, double[3]);

void Suppliment(int*,int***);

void Backup_init(double* , double** , double** ,double* ,double* , double* , double*, double*, int* );

void Backup(int ,double*, double*, double**, double**,double*,double*);

void Parallelize_Geometry();

void pressure_bodyforce_change();

void Comput_Perm_LOCAL(double* ,double** ,double* ,double* ,int );

void pressure_capillary();

//==========Least Square Fitting=========================
void Least_Square_Rel_Perm(double, double);
//===============================================

//=============Rel_Perm_Imb_Drai======================
void Rel_Perm_Imb_Dra(double*);

void output_psi_rel_perm(double ,double* ,int***);
//===============================================


//=============Psi mixture injection======================
void psi_mixture_ini(float* );

//===============================================


//===========input update==============
void input_update();
char inp_up[128];
int update_fre;
//=====================================


const int e[19][3]=
{{0,0,0},{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1},{1,1,0},{-1,-1,0},{1,-1,0},{-1,1,0},{1,0,1},
{-1,0,-1},{1,0,-1},{-1,0,1},{0,1,1},{0,-1,-1},{0,1,-1},{0,-1,1}};

double elat[19][3]=
{{0,0,0},{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1},{1,1,0},{-1,-1,0},{1,-1,0},{-1,1,0},{1,0,1},
{-1,0,-1},{1,0,-1},{-1,0,1},{0,1,1},{0,-1,-1},{0,1,-1},{0,-1,1}};


const double w[19]={1.0/3.0,1.0/18.0,1.0/18.0,1.0/18.0,1.0/18.0,1.0/18.0,1.0/18.0,1.0/36.0,1.0/36.0,1.0/36.0,1.0/36.0,1.0/36.0,1.0/36.0,1.0/36.0,1.0/36.0,1.0/36.0,1.0/36.0,1.0/36.0,1.0/36.0};


//========*************************===============
//int LR[19]={0,2,1,4,3,6,5,10,9,8,7,14,13,12,11,18,17,16,15};
const int LR[19]={0,2,1,4,3,6,5,8,7,10,9,12,11,14,13,16,15,18,17};
const int FRP[19]={0,0,0,0,0,0,0,1,0,2,0,3,0,4,0,0,0,0,0};
const int FLN[19]={0,0,0,0,0,0,0,0,1,0,2,0,3,0,4,0,0,0,0};
const int RP[5]={1,7,9,11,13};
const int LN[5]={2,8,10,12,14};
//=========================================


int n,nx_l,n_max,in_BC,PerDir,freRe,freDe,freVe,frePsi,Par_Geo,Par_nx,Par_ny,Par_nz;
int Zoom,lattice_v,in_psi_BC,par_per_x,par_per_y,par_per_z;


int wr_per,pre_xp,pre_xn,pre_yp,pre_yn,pre_zp,pre_zn,stab,stab_time,fre_backup,psi_xp,psi_xn,psi_yp;
int psi_yn,per_xn,per_yn,per_zn,ini_buf;
int vel_xp,vel_xn,vel_yp,vel_yn,vel_zp,vel_zn,Sub_BC,Out_Mode,mode_backup_ini,psi_zp,psi_zn,per_xp,per_yp,per_zp;
double in_vis,p_xp,p_xn,p_yp,p_yn,p_zp,p_zn,niu_l,niu_g,ContactAngle_parameter,CapA;
double inivx,inivy,inivz,v_xp,v_xn,v_yp,v_yn,v_zp,v_zn,Re_l,Re_g,Capillary,ini_Sat,var_rho;
double error_Per,Permeability,psi_solid,S_l,gxs,gys,gzs,c_s,c_s2,dx_input,dt_input,lat_c;
int bodyforce_apply;
double p_xp_ori,p_xn_ori,p_yp_ori,p_yn_ori,p_zp_ori,p_zn_ori;

//===============Least Square Fitting=====================
int least_square,num_least_square;
double* Per_l_ls;
double* Per_g_ls;
double rel_per_l_ls,rel_per_g_ls,slopl,slopg;
//================================================

//=============Rel_Perm_Imb_Drai======================
int rel_perm_id_mode,rel_perm_id_ids,rel_perm_id_time,rel_perm_id_cri;
int rel_perm_id_dir,rel_perm_chan_num,rel_perm_ift,input_dynamic;
double* rel_perm_sw;
double pre_rell,pre_relg,vary_ift;
char FileName7[128];
//===============================================


//===============MPI TRANSFER INITIALIZATION=============


double* sendl;
double* sendr;

	double* recvl;
	double* recvr;
	
	
int* sumss;             //fluid nodes number of every partition            [procn+1]	
int* bufinfo;                                   //number of nodes of the partirion that need communiate with current processors
                                                        // 0= no contact with current processor, >0 number of nodes that need to communicate with current processor.
                                                         //start from 1 size procn+1       designed for 19 components for f function transfer
int* com_ind;                          //commu nodes indexs (partition no.)   com_ind[0,start from 0] size new int[com_n]
int* com_loc;                           //mpi commu different nodes starting locations in buffet  arrays                                                         
int** nei_loc;                           //index of 18 neibourghs, nei_loc[3][0] is the first neighbour (e[1][]) of node 3
//-------------------
int* coor;      //start from 1, int [sumss[procind]+1];
//------------------

double** bufsend;                       //send buffet for f functions bufsend[comm_index][number]
double** bufrecv;                       //recv buffet for f functions
double** bufsendrhor;
double** bufrecvrhor;
double** bufsendrhob;
double** bufrecvrhob;

//-----------------
int** buflocsend;                       //exchange commu info, used to locate the data after when it is received from MPI communications
int** buflocrecv;                       //two digital combination index*19+ls ls is the direction of 18 vectors

int* bclx;
int* bcly;
int* bclz;
int* bcrx;
int* bcry;
int* bcrz;
int bclxn,bclyn,bclzn,bcrxn,bcryn,bcrzn;
 int com_n;
	
	
	
	
//=================================================

//=========Saturation movement/displacement========
double Sd_l=0.0;
double Sd_g=0.0;
//=================================================



char outputfile[128]="./";
int NCHAR=128;
	char     filename[128], dummy[128+1],filenamepsi[128], backup_rho[128], backup_velocity[128],backup_psi[128],backup_f[128];
	int      dummyInt;
	
int*** Solid;
int*** Solid2;
float* Psi_local;	
char pfix[128];
int decbin;


int pressure_change,pre_chan_pb,pre_chan_1,pre_chan_2,pre_chan_3,pre_chan_4,pre_chan_5;
double pre_chan_pn1,pre_chan_pn2,pre_chan_pn3,pre_chan_pn4,pre_chan_pn5;
double pre_chan_pp1,pre_chan_pp2,pre_chan_pp3,pre_chan_pp4,pre_chan_pp5;
double pre_chan_f1,pre_chan_f2,pre_chan_f3,pre_chan_f4,pre_chan_f5;
double rel_perm_psi,rel_perm_psi2;

//=======VISCOSITY INTERPOLATION=============
double wl,wg,lg0,l1,l2,g1,g2,delta;
//=====================================

//==========CAPILLARY PRESSURE  APPLY=========
double S_l_r;
int pressure_change2,pre_chan_pb2,interval_pre;
double pre_chan_pns,pre_chan_pps,sat_cri_d,pre_chan_fs;
int chan_no,sat_cri;
int error_sat=0;
int start_pre_n=0;
int num_interval=0;
double pre_in,pre_ip;
char FileName6[128];
int ind_error_sat=0;
//=====================================

//=======MIXTURE PSI INJECTION===========
int mix_psi_thickness;
double protion_w;
int zero_sigma_thickness;
//=======================================



int main(int argc , char *argv [])
{	

MPI :: Init (argc , argv );
MPI_Status status ;

double start , finish,remain;

int rank = MPI :: COMM_WORLD . Get_rank ();
int para_size=MPI :: COMM_WORLD . Get_size ();
//**********************
int mpi_size=para_size;
//***********************



int dif,th,tm,ts;

int tse,the,tme;
double elaps;

double Per_l[3],Per_g[3];
double Per_l_LOCAL[3],Per_g_LOCAL[3];





double v_max,error_Per;

        strcpy(pfix,"./");
        if (argc>2)
                strcpy(pfix,argv[2]);
        


	MPI_Barrier(MPI_COMM_WORLD);
	start = MPI_Wtime();

	//=======input update==============
	strcpy(inp_up,argv[1]);
	
	//cout<<inp_up<<endl;
	//==================================


	if (rank==0)
	{
	ifstream fin(argv[1]);
	                                                fin.getline(dummy, NCHAR);
	fin >> filename;				fin.getline(dummy, NCHAR);
	fin >> filenamepsi;				fin.getline(dummy, NCHAR);
	fin >> NX >> NY >> NZ;				fin.getline(dummy, NCHAR);
	fin >> n_max;					fin.getline(dummy, NCHAR);
	fin >> reso;					fin.getline(dummy, NCHAR);
	fin >> in_BC;					fin.getline(dummy, NCHAR);
	fin >> in_psi_BC;				fin.getline(dummy, NCHAR);
	fin >> gx >> gy >> gz;				fin.getline(dummy, NCHAR);
	fin >> pre_xp >> p_xp >> pre_xn >> p_xn;	fin.getline(dummy, NCHAR);
	fin >> pre_yp >> p_yp >> pre_yn >> p_yn;	fin.getline(dummy, NCHAR);
	fin >> pre_zp >> p_zp >> pre_zn >> p_zn;	fin.getline(dummy, NCHAR);
	fin >> vel_xp >> v_xp >> vel_xn >> v_xn;	fin.getline(dummy, NCHAR);
	fin >> vel_yp >> v_yp >> vel_yn >> v_yn;	fin.getline(dummy, NCHAR);
	fin >> vel_zp >> v_zp >> vel_zn >> v_zn;	fin.getline(dummy, NCHAR);
	fin >> psi_xp >> psi_xn;			fin.getline(dummy, NCHAR);
	fin >> psi_yp >> psi_yn;			fin.getline(dummy, NCHAR);
	fin >> psi_zp >> psi_zn;			fin.getline(dummy, NCHAR);
	fin >> niu_l;					fin.getline(dummy, NCHAR);
	fin >> niu_g;					fin.getline(dummy, NCHAR);
	fin >> ContactAngle_parameter;			fin.getline(dummy, NCHAR);
	fin >> CapA;					fin.getline(dummy, NCHAR);
	fin >> inivx >> inivy >> inivz;			fin.getline(dummy, NCHAR);
	fin >> Permeability;				fin.getline(dummy, NCHAR);
							fin.getline(dummy, NCHAR);
	fin >> wr_per;					fin.getline(dummy, NCHAR);
	fin >> PerDir;					fin.getline(dummy, NCHAR);
	fin >> freRe;					fin.getline(dummy, NCHAR);
	fin >> Out_Mode;				fin.getline(dummy, NCHAR);
	fin >> freVe;					fin.getline(dummy, NCHAR);
	fin >> freDe;					fin.getline(dummy, NCHAR);
	fin >> frePsi;					fin.getline(dummy, NCHAR);
							fin.getline(dummy, NCHAR);
	fin >> lattice_v >> dx_input >> dt_input;	fin.getline(dummy, NCHAR);
	fin >> outputfile;				fin.getline(dummy, NCHAR);
	fin >> Sub_BC;					fin.getline(dummy, NCHAR);
	fin >> stab >> stab_time;			fin.getline(dummy, NCHAR);
	fin >> ini_Sat;                                 fin.getline(dummy, NCHAR);
	fin >> ini_buf;                                 fin.getline(dummy, NCHAR);
	fin >> rel_perm_psi>>rel_perm_psi2;				fin.getline(dummy, NCHAR);
	fin >> par_per_x >> par_per_y >>par_per_z;	fin.getline(dummy, NCHAR);
	fin >> per_xp >> per_xn;			fin.getline(dummy, NCHAR);
	fin >> per_yp >> per_yn;			fin.getline(dummy, NCHAR);
	fin >> per_zp >> per_zn;			fin.getline(dummy, NCHAR);
	                                      		fin.getline(dummy, NCHAR);
	fin >> fre_backup;                        	fin.getline(dummy, NCHAR);
	fin >>mode_backup_ini;                		fin.getline(dummy, NCHAR);
							fin.getline(dummy, NCHAR);
	fin >> decbin;					fin.getline(dummy, NCHAR);
							fin.getline(dummy, NCHAR);
	fin >> pressure_change >>pre_chan_pb;		fin.getline(dummy, NCHAR);
	fin >> pre_chan_1>> pre_chan_pn1 >> pre_chan_pp1>>pre_chan_f1;	fin.getline(dummy, NCHAR);
	fin >> pre_chan_2>> pre_chan_pn2 >> pre_chan_pp2>>pre_chan_f2;	fin.getline(dummy, NCHAR);
	fin >> pre_chan_3>> pre_chan_pn3 >> pre_chan_pp3>>pre_chan_f3;	fin.getline(dummy, NCHAR);
	fin >> pre_chan_4>> pre_chan_pn4 >> pre_chan_pp4>>pre_chan_f4;	fin.getline(dummy, NCHAR);
	fin >> pre_chan_5>> pre_chan_pn5 >> pre_chan_pp5>>pre_chan_f5;	fin.getline(dummy, NCHAR);
	fin >> bodyforce_apply;					fin.getline(dummy, NCHAR);
	fin.getline(dummy, NCHAR);        
	
	//=============CAPILLARY PRESSUR======                                                                                                                
	fin >> pressure_change2 >> pre_chan_pb2 >> interval_pre;					fin.getline(dummy, NCHAR);
	fin >> pre_chan_pns >> pre_chan_pps >> pre_chan_fs >>chan_no >> sat_cri;                 fin.getline(dummy, NCHAR);
	fin >> sat_cri_d;                                                                                fin.getline(dummy, NCHAR);
	//=======================================================================
	
	
	//=======================Least Square Fitting====================
	fin.getline(dummy, NCHAR);
	fin >> least_square >> num_least_square;                          fin.getline(dummy, NCHAR);
	//=======================================================
	
	
	//=======================Rel_Perm_Imb_Drai====================
	fin.getline(dummy, NCHAR);
	fin >>rel_perm_id_dir>>rel_perm_chan_num;       fin.getline(dummy, NCHAR);
	fin >>rel_perm_ift >> vary_ift;       		fin.getline(dummy, NCHAR);
	//=============================================================
	

	//================INPUT UPDATE===========================
	fin >> input_dynamic>>update_fre;           		fin.getline(dummy, NCHAR);
	//=======================================================

	//================MIXTURE PSI INJECTION==================
	fin.getline(dummy, NCHAR);
	fin >> mix_psi_thickness;				fin.getline(dummy, NCHAR);
	fin >> protion_w;                                        fin.getline(dummy, NCHAR);
	fin >> zero_sigma_thickness;				fin.getline(dummy, NCHAR);
	//=======================================================
	
	
//	fin >> backup_rho;                        	fin.getline(dummy, NCHAR);
//	fin >> backup_velocity;                		fin.getline(dummy, NCHAR);
//	fin >> backup_psi;                        	fin.getline(dummy, NCHAR);
//	fin >> backup_f;                        	fin.getline(dummy, NCHAR);
	fin.close();
	
	cout<<input_dynamic<<"    asdfa    "<<update_fre<<endl;
	NX=NX-1;NY=NY-1;NZ=NZ-1;
	}

	MPI_Bcast(&filename,128,MPI_CHAR,0,MPI_COMM_WORLD);
	MPI_Bcast(&NX,1,MPI_INT,0,MPI_COMM_WORLD);MPI_Bcast(&var_rho,1,MPI_DOUBLE,0,MPI_COMM_WORLD);
	MPI_Bcast(&NY,1,MPI_INT,0,MPI_COMM_WORLD);MPI_Bcast(&ini_buf,1,MPI_INT,0,MPI_COMM_WORLD);
	MPI_Bcast(&NZ,1,MPI_INT,0,MPI_COMM_WORLD);
	MPI_Bcast(&n_max,1,MPI_INT,0,MPI_COMM_WORLD);MPI_Bcast(&reso,1,MPI_DOUBLE,0,MPI_COMM_WORLD);
	MPI_Bcast(&in_BC,1,MPI_INT,0,MPI_COMM_WORLD);MPI_Bcast(&gx,1,MPI_DOUBLE,0,MPI_COMM_WORLD);
	MPI_Bcast(&gy,1,MPI_DOUBLE,0,MPI_COMM_WORLD);MPI_Bcast(&gz,1,MPI_DOUBLE,0,MPI_COMM_WORLD);
	MPI_Bcast(&pre_xp,1,MPI_INT,0,MPI_COMM_WORLD);MPI_Bcast(&p_xp,1,MPI_DOUBLE,0,MPI_COMM_WORLD);
	MPI_Bcast(&pre_xn,1,MPI_INT,0,MPI_COMM_WORLD);MPI_Bcast(&p_xn,1,MPI_DOUBLE,0,MPI_COMM_WORLD);

	MPI_Bcast(&pre_yp,1,MPI_INT,0,MPI_COMM_WORLD);MPI_Bcast(&p_yp,1,MPI_DOUBLE,0,MPI_COMM_WORLD);
	MPI_Bcast(&pre_yn,1,MPI_INT,0,MPI_COMM_WORLD);MPI_Bcast(&p_yn,1,MPI_DOUBLE,0,MPI_COMM_WORLD);
	MPI_Bcast(&pre_zp,1,MPI_INT,0,MPI_COMM_WORLD);MPI_Bcast(&p_zp,1,MPI_DOUBLE,0,MPI_COMM_WORLD);
	MPI_Bcast(&pre_zn,1,MPI_INT,0,MPI_COMM_WORLD);MPI_Bcast(&p_zn,1,MPI_DOUBLE,0,MPI_COMM_WORLD);
	MPI_Bcast(&vel_xp,1,MPI_INT,0,MPI_COMM_WORLD);MPI_Bcast(&v_xp,1,MPI_DOUBLE,0,MPI_COMM_WORLD);
	MPI_Bcast(&vel_xn,1,MPI_INT,0,MPI_COMM_WORLD);MPI_Bcast(&v_xn,1,MPI_DOUBLE,0,MPI_COMM_WORLD);
	MPI_Bcast(&vel_yp,1,MPI_INT,0,MPI_COMM_WORLD);MPI_Bcast(&v_yp,1,MPI_DOUBLE,0,MPI_COMM_WORLD);
	MPI_Bcast(&vel_yn,1,MPI_INT,0,MPI_COMM_WORLD);MPI_Bcast(&v_yn,1,MPI_DOUBLE,0,MPI_COMM_WORLD);
	MPI_Bcast(&vel_zp,1,MPI_INT,0,MPI_COMM_WORLD);MPI_Bcast(&v_zp,1,MPI_DOUBLE,0,MPI_COMM_WORLD);
	MPI_Bcast(&vel_zn,1,MPI_INT,0,MPI_COMM_WORLD);MPI_Bcast(&v_zn,1,MPI_DOUBLE,0,MPI_COMM_WORLD);

	MPI_Bcast(&inivx,1,MPI_DOUBLE,0,MPI_COMM_WORLD);MPI_Bcast(&inivy,1,MPI_DOUBLE,0,MPI_COMM_WORLD);
	MPI_Bcast(&inivz,1,MPI_DOUBLE,0,MPI_COMM_WORLD);MPI_Bcast(&wr_per,1,MPI_INT,0,MPI_COMM_WORLD);
	MPI_Bcast(&PerDir,1,MPI_INT,0,MPI_COMM_WORLD);MPI_Bcast(&freRe,1,MPI_INT,0,MPI_COMM_WORLD);
	MPI_Bcast(&freVe,1,MPI_INT,0,MPI_COMM_WORLD);MPI_Bcast(&freDe,1,MPI_INT,0,MPI_COMM_WORLD);
	MPI_Bcast(&mir,1,MPI_INT,0,MPI_COMM_WORLD);MPI_Bcast(&CapA,1,MPI_DOUBLE,0,MPI_COMM_WORLD);
	MPI_Bcast(&Zoom,1,MPI_INT,0,MPI_COMM_WORLD);MPI_Bcast(&outputfile,128,MPI_CHAR,0,MPI_COMM_WORLD);
	MPI_Bcast(&Sub_BC,1,MPI_INT,0,MPI_COMM_WORLD);MPI_Bcast(&Out_Mode,1,MPI_INT,0,MPI_COMM_WORLD);
	MPI_Bcast(&niu_l,1,MPI_DOUBLE,0,MPI_COMM_WORLD);MPI_Bcast(&niu_g,1,MPI_DOUBLE,0,MPI_COMM_WORLD);
      	MPI_Bcast(&ContactAngle_parameter,1,MPI_DOUBLE,0,MPI_COMM_WORLD);
	MPI_Bcast(&Permeability,1,MPI_DOUBLE,0,MPI_COMM_WORLD);MPI_Bcast(&frePsi,1,MPI_INT,0,MPI_COMM_WORLD);
	MPI_Bcast(&stab,1,MPI_INT,0,MPI_COMM_WORLD);MPI_Bcast(&stab_time,1,MPI_INT,0,MPI_COMM_WORLD);
	MPI_Bcast(&fre_backup,1,MPI_INT,0,MPI_COMM_WORLD);
	MPI_Bcast(&backup_rho,128,MPI_CHAR,0,MPI_COMM_WORLD);MPI_Bcast(&backup_velocity,128,MPI_CHAR,0,MPI_COMM_WORLD);
	MPI_Bcast(&mode_backup_ini,1,MPI_INT,0,MPI_COMM_WORLD);MPI_Bcast(&backup_psi,128,MPI_CHAR,0,MPI_COMM_WORLD);
	MPI_Bcast(&backup_f,128,MPI_CHAR,0,MPI_COMM_WORLD);

	MPI_Bcast(&lattice_v,1,MPI_INT,0,MPI_COMM_WORLD);MPI_Bcast(&dx_input,1,MPI_DOUBLE,0,MPI_COMM_WORLD);
	MPI_Bcast(&dt_input,1,MPI_DOUBLE,0,MPI_COMM_WORLD);MPI_Bcast(&ini_Sat,1,MPI_DOUBLE,0,MPI_COMM_WORLD);

	MPI_Bcast(&in_psi_BC,1,MPI_INT,0,MPI_COMM_WORLD);MPI_Bcast(&psi_xp,1,MPI_INT,0,MPI_COMM_WORLD);
	MPI_Bcast(&psi_xn,1,MPI_INT,0,MPI_COMM_WORLD);MPI_Bcast(&psi_yp,1,MPI_INT,0,MPI_COMM_WORLD);
	MPI_Bcast(&psi_yn,1,MPI_INT,0,MPI_COMM_WORLD);MPI_Bcast(&psi_zp,1,MPI_INT,0,MPI_COMM_WORLD);
	MPI_Bcast(&psi_zn,1,MPI_INT,0,MPI_COMM_WORLD);

	
	MPI_Bcast(&par_per_x,1,MPI_INT,0,MPI_COMM_WORLD);MPI_Bcast(&par_per_y,1,MPI_INT,0,MPI_COMM_WORLD);
	MPI_Bcast(&par_per_z,1,MPI_INT,0,MPI_COMM_WORLD);
	MPI_Bcast(&per_yp,1,MPI_INT,0,MPI_COMM_WORLD);MPI_Bcast(&per_xp,1,MPI_INT,0,MPI_COMM_WORLD);
	MPI_Bcast(&per_yn,1,MPI_INT,0,MPI_COMM_WORLD);MPI_Bcast(&per_zp,1,MPI_INT,0,MPI_COMM_WORLD);
	MPI_Bcast(&per_zn,1,MPI_INT,0,MPI_COMM_WORLD);MPI_Bcast(&per_xn,1,MPI_INT,0,MPI_COMM_WORLD);


	MPI_Bcast(&pressure_change,1,MPI_INT,0,MPI_COMM_WORLD);MPI_Bcast(&pre_chan_pb,1,MPI_INT,0,MPI_COMM_WORLD);
	MPI_Bcast(&pre_chan_1,1,MPI_INT,0,MPI_COMM_WORLD);MPI_Bcast(&pre_chan_pn1,1,MPI_DOUBLE,0,MPI_COMM_WORLD);
	MPI_Bcast(&pre_chan_pp1,1,MPI_DOUBLE,0,MPI_COMM_WORLD);MPI_Bcast(&pre_chan_f1,1,MPI_DOUBLE,0,MPI_COMM_WORLD);
	MPI_Bcast(&pre_chan_2,1,MPI_INT,0,MPI_COMM_WORLD);MPI_Bcast(&pre_chan_pn2,1,MPI_DOUBLE,0,MPI_COMM_WORLD);
	MPI_Bcast(&pre_chan_pp2,1,MPI_DOUBLE,0,MPI_COMM_WORLD);MPI_Bcast(&pre_chan_f2,1,MPI_DOUBLE,0,MPI_COMM_WORLD);
	MPI_Bcast(&pre_chan_3,1,MPI_INT,0,MPI_COMM_WORLD);MPI_Bcast(&pre_chan_pn3,1,MPI_DOUBLE,0,MPI_COMM_WORLD);
	MPI_Bcast(&pre_chan_pp3,1,MPI_DOUBLE,0,MPI_COMM_WORLD);MPI_Bcast(&pre_chan_f3,1,MPI_DOUBLE,0,MPI_COMM_WORLD);
	MPI_Bcast(&pre_chan_4,1,MPI_INT,0,MPI_COMM_WORLD);MPI_Bcast(&pre_chan_pn4,1,MPI_DOUBLE,0,MPI_COMM_WORLD);
	MPI_Bcast(&pre_chan_pp4,1,MPI_DOUBLE,0,MPI_COMM_WORLD);MPI_Bcast(&pre_chan_f4,1,MPI_DOUBLE,0,MPI_COMM_WORLD);
	MPI_Bcast(&pre_chan_5,1,MPI_INT,0,MPI_COMM_WORLD);MPI_Bcast(&pre_chan_pn5,1,MPI_DOUBLE,0,MPI_COMM_WORLD);
	MPI_Bcast(&pre_chan_pp5,1,MPI_DOUBLE,0,MPI_COMM_WORLD);MPI_Bcast(&pre_chan_f5,1,MPI_DOUBLE,0,MPI_COMM_WORLD);
	MPI_Bcast(&rel_perm_psi,1,MPI_DOUBLE,0,MPI_COMM_WORLD);MPI_Bcast(&rel_perm_psi2,1,MPI_DOUBLE,0,MPI_COMM_WORLD);
	MPI_Bcast(&bodyforce_apply,1,MPI_INT,0,MPI_COMM_WORLD);

	//=============CAPILLARY PRESSURE APPLY================
	MPI_Bcast(&pressure_change2,1,MPI_INT,0,MPI_COMM_WORLD);MPI_Bcast(&pre_chan_pb2,1,MPI_INT,0,MPI_COMM_WORLD);
	MPI_Bcast(&pre_chan_pps,1,MPI_DOUBLE,0,MPI_COMM_WORLD);MPI_Bcast(&pre_chan_pns,1,MPI_DOUBLE,0,MPI_COMM_WORLD);
	MPI_Bcast(&interval_pre,1,MPI_INT,0,MPI_COMM_WORLD);MPI_Bcast(&chan_no,1,MPI_INT,0,MPI_COMM_WORLD);
	MPI_Bcast(&sat_cri,1,MPI_INT,0,MPI_COMM_WORLD);MPI_Bcast(&sat_cri_d,1,MPI_DOUBLE,0,MPI_COMM_WORLD);
	MPI_Bcast(&pre_chan_fs,1,MPI_DOUBLE,0,MPI_COMM_WORLD);
	if (pre_chan_pb2==1)
	{
	 if (pressure_change2==1)
	         {pre_in=(pre_chan_pns-p_xn)/(double)(chan_no);pre_ip=(pre_chan_pps-p_xp)/(double)(chan_no);}
	 if (pressure_change2==2)
	         {pre_in=(pre_chan_pns-p_yn)/(double)(chan_no);pre_ip=(pre_chan_pps-p_yp)/(double)(chan_no);}
	 if (pressure_change2==3)
	         {pre_in=(pre_chan_pns-p_zn)/(double)(chan_no);pre_ip=(pre_chan_pps-p_zp)/(double)(chan_no);}
	}
	else
	        {
	        if (pressure_change2==1)
                        pre_in=(pre_chan_fs-gx)/(double)(chan_no);
	         if (pressure_change2==2)
	                 pre_in=(pre_chan_fs-gy)/(double)(chan_no);
	         if (pressure_change2==3)
	                 pre_in=(pre_chan_fs-gz)/(double)(chan_no);
	        
	        }
	//==================================================

	//==================Least Square Fitting=====================
	MPI_Bcast(&least_square,1,MPI_INT,0,MPI_COMM_WORLD);
	MPI_Bcast(&num_least_square,1,MPI_INT,0,MPI_COMM_WORLD);
	Per_l_ls= new double[num_least_square];
	Per_g_ls= new double[num_least_square];
	for (int i=0;i<num_least_square;i++)
	        {
	                Per_l_ls[i]=0;
	                Per_g_ls[i]=0;
	        }
	//===================================================
	
	
	//==================Rel_Perm_Imb_Drai=====================
	MPI_Bcast(&rel_perm_id_dir,1,MPI_INT,0,MPI_COMM_WORLD);
	MPI_Bcast(&rel_perm_chan_num,1,MPI_INT,0,MPI_COMM_WORLD);
	MPI_Bcast(&rel_perm_ift,1,MPI_INT,0,MPI_COMM_WORLD);
	MPI_Bcast(&vary_ift,1,MPI_DOUBLE,0,MPI_COMM_WORLD);

	//==============input updat=====================================
	MPI_Bcast(&input_dynamic,1,MPI_INT,0,MPI_COMM_WORLD);
	MPI_Bcast(&update_fre,1,MPI_INT,0,MPI_COMM_WORLD);












	
	rel_perm_sw=new double[rel_perm_chan_num];
	if (rel_perm_id_dir==1)
	        for (int i=0;i<rel_perm_chan_num;i++)
	                rel_perm_sw[i]=(double)(i+1)*(1.0/(rel_perm_chan_num+1));
	 if (rel_perm_id_dir==2)
	        for (int i=0;i<rel_perm_chan_num;i++)
	                rel_perm_sw[i]=(double)(rel_perm_chan_num-i)*(1.0/(rel_perm_chan_num+1));
	//===================================================
	
	
	
	
	//=============MIXTURE PSI INJECTION=================
	MPI_Bcast(&mix_psi_thickness,1,MPI_INT,0,MPI_COMM_WORLD);MPI_Bcast(&protion_w,1,MPI_DOUBLE,0,MPI_COMM_WORLD);
	MPI_Bcast(&zero_sigma_thickness,1,MPI_INT,0,MPI_COMM_WORLD);
	//===================================================


p_xn_ori=p_xn;p_xp_ori=p_xp;
p_yn_ori=p_yn;p_yp_ori=p_yp;
p_zn_ori=p_zn;p_zp_ori=p_zp;






mirX=0;mirY=0;mirZ=0;
mir=1;Zoom=1;


int U_max_ref=0;


if (mirX==1)
	NX=NX*2+1;
if (mirY==1)
	NY=NY*2+1;
if (mirZ==1)
	NZ=NZ*2+1;


if (Zoom>1)
	{	
	NX=(NX+1)*Zoom-1;
	NY=(NY+1)*Zoom-1;
	NZ=(NZ+1)*Zoom-1;
	}




	double* Permia;
	double* rho;
	double* rho_r;
	double* rho_b;
	double* rhor;
	double* rhob;
	double** u;
	double**f;
	double**F;
	double**u0;
	int* SupInv;
	//double* forcex;
	//double* forcey;
	//double* forcez;
	
	double* psi;


	
	int*  Sl;
	int*  Sr;

	

	Parallelize_Geometry();
	
	
	/*
	MPI_Barrier(MPI_COMM_WORLD);
		

	init_Sparse_read_rock_parallel(Sl,Sr);
	MPI_Barrier(MPI_COMM_WORLD);
	*/

	
	//***************************************************
	//WARRING: SPARSE MATRIX STARTS FROM INDEX 1 NOT 0!!!
	//***************************************************

	Permia = new double[3];
	rho = new double[Count+1];
	rho_r = new double[Count+1];
	rho_b = new double[Count+1];
	rhor = new double[Count+1];
	rhob = new double[Count+1];
	psi = new double[Count+1];
	
	u = new double*[Count+1];
	u[0] = new double[(Count+1)*3];
	        for (int i=1;i<=Count;i++)
	              u[i] = u[i-1]+3;
	      
	      
	f = new double*[Count+1];
	f[0] =new double[(Count+1)*19];
	        for (int i=1;i<=Count;i++)
	                f[i] = f[i-1]+19;
	        
	        
	F = new double*[Count+1];
	F[0] =new double[(Count+1)*19];
	for (int i=1;i<=Count;i++)
		F[i] = F[i-1]+19;
	
	
	u0 = new double*[Count+1];
	u0[0] = new double[(Count+1)*3];
	        for (int i=1;i<=Count;i++)
		u0[i] = u0[i-1]+3;
	
	
	SupInv = new int[Count+1];


	Comput_MI(M,MI);
	
	//Suppliment(SupInv,Solid);

	MPI_Barrier(MPI_COMM_WORLD);
	
	if (Out_Mode==1)
		Geometry_Par(Solid);

	if ((freVe>=0) or (freDe>=0))
	{
	
	
		Geometry(Solid);
	}


	if (mode_backup_ini==0)
	        init(rho,u,f,psi,rho_r,rho_b,rhor, rhob,SupInv);
	else
	      Backup_init(rho, u, f,psi,rho_r, rho_b, rhor, rhob,SupInv);  

if (rank==0)
{
        cout<<endl;
        cout<<"INITIALIZATION COMPLETED"<<endl;
        cout<<endl;
}




//========================================================
char FileName[128];strcpy(FileName,outputfile);
char FileName2[128];strcpy(FileName2,outputfile);
char FileName3[128];strcpy(FileName3,outputfile);
char FileName4[128];strcpy(FileName4,outputfile);
char FileName5[128];strcpy(FileName5,outputfile);

strcat(FileName,"Results.txt");
strcat(FileName2,"Relative_Permeability_Component1.txt");
strcat(FileName4,"Relative_Permeability_Component2.txt");
strcat(FileName3,"bodyforce.txt");
strcat(FileName5,"Velocity_ave_max.txt");
//========================================================






ofstream fins;	
	fins.open(FileName,ios::trunc);
	fins.close();
	
	
//===========CAPILLARY PRESSURE APPLY============	
strcpy(FileName6,outputfile);
strcat(FileName6,"Capillary_Pressure.txt");
fins.open(FileName6,ios::trunc);
fins.close();
//=========================================

//===========Rel_Perm_Imb_Drai============	
strcpy(FileName7,outputfile);
strcat(FileName7,"Imb_Drai_Rel_Perm.txt");
fins.open(FileName7,ios::trunc);
fins.close();
//=========================================


if (wr_per==1)
	{
	fins.open(FileName2,ios::trunc);
	fins.close();
	}


	fins.open(FileName3,ios::out);
	fins.close();

	fins.open(FileName4,ios::out);
	fins.close();

	fins.open(FileName5,ios::out);
	fins.close();

	
	
	//=============MPI TRANSFER INITIALIZATION====================
	
	
	
	//=====================================================
	
	
	
MPI_Barrier(MPI_COMM_WORLD);	
	
	for(n=0;n<=n_max;n++)
	{
	
	if ((stab==1) and (n==stab_time))
		{gxs=gx;gys=gy;gzs=gz;}
			
	
	collision(rho,u,f,F,psi,rho_r,rho_b,rhor,rhob,SupInv,Solid,Sl,Sr);
	//cout<<"       qgragf       "<<endl;


	if (pressure_change>0)
		pressure_bodyforce_change();
	
	if (pressure_change2>0)
		pressure_capillary();
	
	
	//=========input update=================fin >> input_dynamic>>update_fre;       
	if (input_dynamic>0)
		if (n%update_fre==0)
		input_update();
	//======================================


	//periodic_streaming(f,F,SupInv,Solid,Sl,Sr,rho,u);	
	
	if ((stab==0) or ((stab==1) and (n>=stab_time)))
	{
	if ((1-pre_xp)*(1-pre_xn)*(1-pre_yp)*(1-pre_yn)*(1-pre_zp)*(1-pre_zn)==0)
		boundary_pressure(pre_xp,p_xp,pre_xn,p_xn,pre_yp,p_yp,pre_yn,p_yn,pre_zp,p_zp,pre_zn,p_zn,f,F,u,rho,Solid);

	
	if ((1-vel_xp)*(1-vel_xn)*(1-vel_yp)*(1-vel_yn)*(1-vel_zp)*(1-vel_zn)==0)
		boundary_velocity(vel_xp,v_xp,vel_xn,v_xn,vel_yp,v_yp,vel_yn,v_yn,vel_zp,v_zp,vel_zn,v_zn,f,F,rho,u,Solid);
	}
  		 
		comput_macro_variables(rho,u,u0,f,F,rho_r,rho_b,rhor,rhob,psi,SupInv);


	//if ((1-pre_xp)*(1-pre_xn)*(1-pre_yp)*(1-pre_yn)*(1-pre_zp)*(1-pre_zn)==0)
	//	boundary_pressure(pre_xp,p_xp,pre_xn,p_xn,pre_yp,p_yp,pre_yn,p_yn,pre_zp,p_zp,pre_zn,p_zn,f,u,rho,Solid);
		
	//if ((1-vel_xp)*(1-vel_xn)*(1-vel_yp)*(1-vel_yn)*(1-vel_zp)*(1-vel_zn)==0)
	//        boundary_velocity(vel_xp,v_xp,vel_xn,v_xn,vel_yp,v_yp,vel_yn,v_yn,vel_zp,v_zp,vel_zn,v_zn,f,rho,u,Solid);
	
	if(n%freRe==0)
		{       
		        
		        
		        //============IFT CHANGE====================rel_perm_ift >> vary_ift;   
			if ((rel_perm_ift>0) and (rel_perm_ift==n))
				CapA=vary_ift;
			//==========================================
			
			if (rank==0)
			{
			ofstream fin(FileName,ios::out);
			fin<<"The"<<n-freRe<<"th computation result:"<<endl;
			Re_l=u_ave*(NY+1)/niu_l;Re_g=u_ave*(NY+1)/niu_g;
		        fin<<"The Maximum velocity is: "<<setprecision(6)<<u_max<<"   Re_l="<<Re_l<<"   Re_g="<<Re_g<<endl;
			fin<<"Courant Number="<<u_max*dt/dx<<"	 Capillary Num="<<Capillary<<endl;
			fin<<"The max relative error of velocity is: "
				<<setiosflags(ios::scientific)<<error<<endl;
			fin<<"The relative permeability of component 1 is "<<Per_l[0]*reso*reso*1000/Permeability<<", "<<Per_l[1]*reso*reso*1000/Permeability<<", "<<Per_l[2]*reso*reso*1000/Permeability<<endl;
			fin<<"The relative permeability of component 2 is "<<Per_g[0]*reso*reso*1000/Permeability<<", "<<Per_g[1]*reso*reso*1000/Permeability<<", "<<Per_g[2]*reso*reso*1000/Permeability<<endl;
			fin<<"The LEAST SQUARED relative permeability of component 1 is "<<rel_per_l_ls<<endl;
			fin<<"The LEAST SQUARED relative permeability of component 2 is "<<rel_per_g_ls<<endl;
			fin<<"Satuation of Component 1: "<<S_l<<", "<<"The satuation of Component 2: "<<1-S_l<<endl;
			fin<<"The relative error of permiability computing is: "<<error_Per<<endl;
			fin<<"Elapsed time is "<< the<<"h"<<tme<<"m"<<tse<<"s"<<endl;
			fin<<"The expected completion time is "<<th<<"h"<<tm<<"m"<<ts<<"s"<<endl;
			fin<<endl;
			fin.close();
			}
			
			
			pre_u_ave=u_ave;
			error=Error(u,u0,&u_max,&u_ave);
			if (u_max>=10.0)	U_max_ref+=1;
			
			error_Per=Comput_Perm(psi,u,Per_l,Per_g,PerDir,SupInv);
			
			//==========CAPILLARY PRESSURE  APPLY=========
			S_l_r=S_l;
			//=====================================
			
			S_l=Comput_Saturation(psi,Solid,SupInv);


			//=========Comput Saturation Displacement=========
			//Sd_l=Comput_Saturation_disp(psi,Solid,SupInv);
			//================================================
			
			//=============Rel_Perm_Imb_Drai=================
			pre_rell=rel_per_l_ls;
			pre_relg=rel_per_g_ls;
			//===========================================
			


			
			//=====================Least Sqare Fitting======================
			if (least_square>0)
			        if (PerDir==1)
			        Least_Square_Rel_Perm(Per_l[0]*reso*reso*1000/Permeability,Per_g[0]*reso*reso*1000/Permeability);
			        else
			                if (PerDir==2)
			                Least_Square_Rel_Perm(Per_l[1]*reso*reso*1000/Permeability,Per_g[1]*reso*reso*1000/Permeability);
			                else
			                        Least_Square_Rel_Perm(Per_l[2]*reso*reso*1000/Permeability,Per_g[2]*reso*reso*1000/Permeability);
			    MPI_Bcast(&rel_per_l_ls,1,MPI_DOUBLE,0,MPI_COMM_WORLD); 
			    MPI_Bcast(&rel_per_g_ls,1,MPI_DOUBLE,0,MPI_COMM_WORLD); 
			   //======================================================== 
			  
			   
	

			 
			 //=============Rel_Perm_Imb_Drai=================        
			 if (rel_perm_id_dir>0)
			         Rel_Perm_Imb_Dra(psi);               
			 //===========================================
			 
			 
			 
			 
			 
			if (rank==0)
			{
			ofstream fin(FileName,ios::app);
			finish = MPI_Wtime();

			fin<<"The"<<n<<"th computation result:"<<endl;

			Re_l=u_ave*(NY+1)/niu_l;Re_g=u_ave*(NY+1)/niu_g;
		//=============================================================================================
		        remain=(n_max-n)*((finish-start)/n);
			th=int(remain/3600);
			tm=int((remain-th*3600)/60);
			ts=int(remain-(th*3600+tm*60));

			elaps=finish-start;
			the=int(elaps/3600);
			tme=int((elaps-the*3600)/60);
			tse=int(elaps-(the*3600+tme*60));

		//==============================================================================================
		        fin<<"The Maximum velocity is: "<<setprecision(6)<<u_max<<"   Re_l="<<Re_l<<"   Re_g="<<Re_g<<endl;
			fin<<"Courant Number="<<u_max*dt/dx<<"	 Capillary Num="<<Capillary<<endl;
		//===============================================================================================
			fin<<"The max relative error of velocity is: "
				<<setiosflags(ios::scientific)<<error<<endl;
			fin<<"The relative permeability of component 1 is "<<Per_l[0]*reso*reso*1000/Permeability<<", "<<Per_l[1]*reso*reso*1000/Permeability<<", "<<Per_l[2]*reso*reso*1000/Permeability<<endl;
			fin<<"The relative permeability of component 2 is "<<Per_g[0]*reso*reso*1000/Permeability<<", "<<Per_g[1]*reso*reso*1000/Permeability<<", "<<Per_g[2]*reso*reso*1000/Permeability<<endl;
	
			fin<<"The LEAST SQUARED relative permeability of component 1 is "<<rel_per_l_ls<<endl;
			fin<<"The LEAST SQUARED relative permeability of component 2 is "<<rel_per_g_ls<<endl;
			fin<<"Satuation of Component 1: "<<S_l<<", "<<"The satuation of Component 2: "<<1-S_l<<endl;
			fin<<"The relative error of permiability computing is: "<<error_Per<<endl;
			fin<<"Elapsed time is "<< the<<"h"<<tme<<"m"<<tse<<"s"<<endl;
			fin<<"The expected completion time is "<<th<<"h"<<tm<<"m"<<ts<<"s"<<endl;
			fin<<endl;
			fin.close();
		//==============================================================================================
			
		if (wr_per==1)
			{  
			ofstream finfs(FileName2,ios::app);
			switch(PerDir)
				{
				case 1:
				finfs<<Per_l[0]*reso*reso*1000/Permeability<<" "<<S_l<<" "<<Per_l_LOCAL[0]*reso*reso*1000/Permeability<<" "<<1-S_l<<" "<<p_xn<<" "<<p_xp<<" "<<gxs<<" "<<rel_per_l_ls<<endl;break;
				case 2:
				finfs<<Per_l[1]*reso*reso*1000/Permeability<<" "<<S_l<<" "<<Per_l_LOCAL[1]*reso*reso*1000/Permeability<<" "<<1-S_l<<" "<<p_yn<<" "<<p_yp<<" "<<gys<<" "<<rel_per_l_ls<<endl;break;
				case 3:
				finfs<<Per_l[2]*reso*reso*1000/Permeability<<" "<<S_l<<" "<<Per_l_LOCAL[2]*reso*reso*1000/Permeability<<" "<<1-S_l<<" "<<p_zn<<" "<<p_zp<<" "<<gzs<<" "<<rel_per_l_ls<<endl;break;
				default:
				finfs<<Per_l[0]*reso*reso*1000/Permeability<<" "<<S_l<<" "<<Per_l_LOCAL[0]*reso*reso*1000/Permeability<<" "<<1-S_l<<" "<<p_xn<<" "<<p_xp<<" "<<gxs<<" "<<rel_per_l_ls<<endl;break;
				}
			finfs.close();

		ofstream finfs2(FileName4,ios::app);
			switch(PerDir)
				{
				case 1:
				finfs2<<Per_g[0]*reso*reso*1000/Permeability<<" "<<1-S_l<<" "<<Per_g_LOCAL[0]*reso*reso*1000/Permeability<<" "<<S_l<<" "<<rel_per_g_ls<<" "<<slopl<<" "<<endl;break;
				case 2:
				finfs2<<Per_g[1]*reso*reso*1000/Permeability<<" "<<1-S_l<<" "<<Per_g_LOCAL[1]*reso*reso*1000/Permeability<<" "<<S_l<<" "<<rel_per_g_ls<<" "<<slopl<<" "<<endl;break;
				case 3:
				finfs2<<Per_g[2]*reso*reso*1000/Permeability<<" "<<1-S_l<<" "<<Per_g_LOCAL[2]*reso*reso*1000/Permeability<<" "<<S_l<<" "<<rel_per_g_ls<<" "<<slopl<<" "<<endl;break;
				default:
				finfs2<<Per_g[0]*reso*reso*1000/Permeability<<" "<<1-S_l<<" "<<Per_g_LOCAL[0]*reso*reso*1000/Permeability<<" "<<S_l<<" "<<rel_per_g_ls<<" "<<slopl<<" "<<endl;break;
				}
			finfs2.close();
			}

			//==========for bodyforce output===========
			ofstream finf3(FileName3,ios::app);
			finf3<<S_l<<" "<<1-S_l<<" "<<abs((S_l_r-S_l)/(S_l+1e-20))<<" "<<Sd_l<<" "<<Sd_g<<endl;
			finf3.close();
			ofstream finf5(FileName5,ios::app);
			finf5<<u_ave<<" "<<u_max<<"  "<<error<<"  "<<abs((pre_u_ave-u_ave)/(u_ave+1e-20))<<endl;
			finf5.close();

			

		//==============================================================================================
			cout<<"The"<<n<<"th computation result:"<<endl;
			cout<<"The Density of point(NX/2,NY/2,NZ/2) is: "<<setprecision(6)
				<<rho[(int)(Count/2)]<<endl;
			
			cout<<"The Maximum velocity is: "<<setprecision(6)<<u_max<<"   Re_l="<<Re_l<<"   Re_g="<<Re_g<<endl;
			cout<<"Courant Number="<<u_max*dt/dx<<"	 Capillary Num="<<Capillary<<endl;
			cout<<"The max relative error of uv is: "
				<<setiosflags(ios::scientific)<<error<<endl;
			cout<<"The relative permeability of component 1 is "<<Per_l[0]*reso*reso*1000/Permeability<<", "<<Per_l[1]*reso*reso*1000/Permeability<<", "<<Per_l[2]*reso*reso*1000/Permeability<<endl;
			cout<<"The relative permeability of component 2 is "<<Per_g[0]*reso*reso*1000/Permeability<<", "<<Per_g[1]*reso*reso*1000/Permeability<<", "<<Per_g[2]*reso*reso*1000/Permeability<<endl;
			
			cout<<"The LEAST SQUARED relative permeability of component 1 is "<<rel_per_l_ls<<endl;
			cout<<"The LEAST SQUARED relative permeability of component 2 is "<<rel_per_g_ls<<endl;
			cout<<"Satuation of Component 1: "<<S_l<<", "<<"The satuation of Component 2: "<<1-S_l<<endl;
			cout<<"The relative error of permiability computing is: "<<error_Per<<endl;
			cout<<"Elapsed time is "<< the<<"h"<<tme<<"m"<<tse<<"s"<<endl;
			cout<<"The expected completion time is "<<th<<"h"<<tm<<"m"<<ts<<"s"<<endl;
			cout<<endl;
			}
			
			if ((freDe>0) and (n%freDe==0) and (n>0))
					output_density(n,rho,mirX,mirY,mirZ,mir,Solid);
			

			if ((freVe>0) and (n%freVe==0) and (n>0))
					output_velocity(n,rho,u,mirX,mirY,mirZ,mir,Solid);
				
			
			//===================================	
			if ((frePsi>=0) and (n%frePsi==0))
					output_psi(n,psi,mirX,mirY,mirZ,mir,Solid);
			
			//===================================
			
			if ((fre_backup>0) and (n%fre_backup==0)  and (n>0))
			        Backup(n,rho,psi,u,f,rho_r,rho_b);
			
			
			//if(error!=error) {cout<<"PROGRAM STOP"<<endl;break;};
			if(U_max_ref>=5) {cout<<"PROGRAM STOP DUE TO HIGH VELOCITY"<<endl;break;}
		}	
	}

	
	
	if (fre_backup>=0)
			        Backup(n_max,rho,psi,u,f,rho_r,rho_b);

	MPI_Barrier(MPI_COMM_WORLD);

	


	finish = MPI_Wtime();

	MPI_Barrier(MPI_COMM_WORLD);
	if(rank==0)
			{
    			cout<<"Elapsed time is "<< the<<"h"<<tme<<"m"<<tse<<"s"<<endl;
    			cout<<"Accuracy: "<<MPI_Wtick()<<" Second"<<endl;
			ofstream fin(FileName,ios::app);
			fin<<"Elapsed time is "<< the<<"h"<<tme<<"m"<<tse<<"s"<<endl;
			fin<<"Accuracy: "<<MPI_Wtick()<<" Second"<<endl;
			}

	MPI :: Finalize ();

	
}

//=============Rel_Perm_Imb_Drai======================
//int rel_perm_id_mode,rel_perm_id_ids,rel_perm_id_cri;
//int rel_perm_id_dir,rel_perm_chan_num;
//double* rel_perm_sw;
//double pre_rell,pre_relg;cri_rells
//===============================================

void Rel_Perm_Imb_Dra(double* psi)
{
        
        
        int rank = MPI :: COMM_WORLD . Get_rank ();
	int mpi_size=MPI :: COMM_WORLD . Get_size ();
	
	
       
        if (rel_perm_id_mode==1)
                {
                       
                        if ((rel_perm_id_dir==1) and (S_l>rel_perm_sw[rel_perm_id_ids]))
                        {
                                rel_perm_id_mode=2;
                               
                        }
                        
                      if ((rel_perm_id_dir==2) and (S_l<rel_perm_sw[rel_perm_id_ids]))
                        {
                                rel_perm_id_mode=2;
                               
                        }          
                        
                }
                
            if (rel_perm_id_mode==2)
            {
                    rel_perm_id_time+=1;
                    
                    //=============export psi=================================
                    //if ((rel_perm_id_time>=num_least_square) and (rel_perm_output_psi==1))
                    //{
                            
                     //export psi  ouput_psi_rel_perm
                     if (rel_perm_id_ids<=rel_perm_chan_num-1)
                     //output_psi_rel_perm(rel_perm_sw[rel_perm_id_ids],psi,Solid);
                     output_psi(n,psi,mirX,mirY,mirZ,mir,Solid);



                     rel_perm_id_ids+=1;
                                if (rel_perm_id_ids<rel_perm_chan_num) 
                                        {
                                           rel_perm_id_mode=1;
                                           in_psi_BC=1;
                                           ini_Sat=rel_perm_sw[rel_perm_id_ids];
                                           
                                           /*
                                           if (rel_perm_bodyf_mode==1)
                                           {
                                                   if (PerDir==1)
                                                           gxs=rel_perm_bodyf;
                                                   if (PerDir==2)
                                                           gys=rel_perm_bodyf;
                                                   if (PerDir==3)
                                                           gzs=rel_perm_bodyf;
                                        }
                                        */
                                        
                                        
                                        }
                                        
                    //}             
             //===================================================
             
             
             
            }
            
  
}



void Least_Square_Rel_Perm(double per_l, double per_g)
{
        
        double sum_l,sum_g,xsum,x1_l,x1_g,sum2_l,sum2_g,sumb,x0_l,x0_g;
       
        for (int i=num_least_square-1;i>0;i--)
        {        
                Per_l_ls[i]=Per_l_ls[i-1];
                Per_g_ls[i]=Per_g_ls[i-1];
        }
        Per_l_ls[0]=per_l;
        Per_g_ls[0]=per_g;
        
        xsum=0.0;sum_l=0.0;sum_g=0.0;
       for (int i=0;i<num_least_square;i++)
               {
                       xsum+=(double) i;
                       sum_l+=Per_l_ls[i];
                       sum_g+=Per_g_ls[i];
               }
        
               sum_l/=(double)num_least_square;
               sum_g/=(double)num_least_square;
               xsum/=(double)num_least_square;
            
               
            sum2_l=0.0;sum2_g=0.0;sumb=0.0;
            for (int i=0;i<num_least_square;i++)
                    {
                            sum2_l+=(i-xsum)*(Per_l_ls[i]-sum_l);
                            sum2_g+=(i-xsum)*(Per_g_ls[i]-sum_g);
                            sumb+=(i-xsum)*(i-xsum);
                    } 
                    
                    x1_l=sum2_l/sumb;
                    x1_g=sum2_g/sumb;
                    x0_l=sum_l-x1_l*xsum;
                    x0_g=sum_g-x1_g*xsum;
                    
                    rel_per_l_ls=x0_l;
                    rel_per_g_ls=x0_g;
                    
                    slopl=x1_l;slopg=x1_g;
                    
                   
                    
}




void pressure_bodyforce_change()
{

	int rank = MPI :: COMM_WORLD . Get_rank ();
	int mpi_size=MPI :: COMM_WORLD . Get_size ();

if (pressure_change==1)
		{
		if (n<=pre_chan_1)
			if (pre_chan_pb==1)
			{
			p_xn=p_xn_ori+(pre_chan_pn1-p_xn_ori)*(1-(double)(pre_chan_1-n)/pre_chan_1);
			p_xp=p_xp_ori+(pre_chan_pp1-p_xp_ori)*(1-(double)(pre_chan_1-n)/pre_chan_1);
			//if (rank==0)
			//cout<<p_xn_ori<<"   "<<pre_chan_pn1<<"  "<<p_xn<<endl;
			}
			else
				gxs=gx+(pre_chan_f1-gx)*(1-(double)(pre_chan_1-n)/pre_chan_1);

		if ((n<=pre_chan_2) and (n>pre_chan_1))
			if (pre_chan_pb==1)
			{
			
			p_xn=pre_chan_pn1+(pre_chan_pn2-pre_chan_pn1)*(1-(double)(pre_chan_2-n)/(pre_chan_2-pre_chan_1));
			p_xp=pre_chan_pp1+(pre_chan_pp2-pre_chan_pp1)*(1-(double)(pre_chan_2-n)/(pre_chan_2-pre_chan_1));
			}
			else
				gxs=pre_chan_f1+(pre_chan_f2-pre_chan_f1)*(1-(double)(pre_chan_2-n)/(pre_chan_2-pre_chan_1));

		if ((n<=pre_chan_3) and (n>pre_chan_2))
			if (pre_chan_pb==1)
			{
			
			p_xn=pre_chan_pn2+(pre_chan_pn3-pre_chan_pn2)*(1-(double)(pre_chan_3-n)/(pre_chan_3-pre_chan_2));
			p_xp=pre_chan_pp2+(pre_chan_pp3-pre_chan_pp2)*(1-(double)(pre_chan_3-n)/(pre_chan_3-pre_chan_2));
			}
			else
				gxs=pre_chan_f2+(pre_chan_f3-pre_chan_f2)*(1-(double)(pre_chan_3-n)/(pre_chan_3-pre_chan_2));

		if ((n<=pre_chan_4) and (n>pre_chan_3))
			if (pre_chan_pb==1)
			{
			p_xn=pre_chan_pn3+(pre_chan_pn4-pre_chan_pn3)*(1-(double)(pre_chan_4-n)/(pre_chan_4-pre_chan_3));
			p_xp=pre_chan_pp3+(pre_chan_pp4-pre_chan_pp3)*(1-(double)(pre_chan_4-n)/(pre_chan_4-pre_chan_3));
			}
			else
				gxs=pre_chan_f3+(pre_chan_f4-pre_chan_f3)*(1-(double)(pre_chan_4-n)/(pre_chan_4-pre_chan_3));

		if ((n<=pre_chan_5) and (n>pre_chan_3))
			if (pre_chan_pb==1)
			{
			p_xn=pre_chan_pn4+(pre_chan_pn5-pre_chan_pn4)*(1-(double)(pre_chan_5-n)/(pre_chan_5-pre_chan_4));
			p_xp=pre_chan_pp4+(pre_chan_pp5-pre_chan_pp4)*(1-(double)(pre_chan_5-n)/(pre_chan_5-pre_chan_4));
			}
			else
				gxs=pre_chan_f4+(pre_chan_f5-pre_chan_f4)*(1-(double)(pre_chan_5-n)/(pre_chan_5-pre_chan_4));


	 
		}

if (pressure_change==2)
		{
		if (n<=pre_chan_1)
			if (pre_chan_pb==1)
			{
			p_yn=p_yn_ori+(pre_chan_pn1-p_yn_ori)*(1-(double)(pre_chan_1-n)/pre_chan_1);
			p_yp=p_yp_ori+(pre_chan_pp1-p_yp_ori)*(1-(double)(pre_chan_1-n)/pre_chan_1);
			}
			else
				gys=gx+(pre_chan_f1-gy)*(1-(double)(pre_chan_1-n)/pre_chan_1);

		if ((n<=pre_chan_2) and (n>pre_chan_1))
			if (pre_chan_pb==1)
			{
			
			p_yn=pre_chan_pn1+(pre_chan_pn2-pre_chan_pn1)*(1-(double)(pre_chan_2-n)/(pre_chan_2-pre_chan_1));
			p_yp=pre_chan_pp1+(pre_chan_pp2-pre_chan_pp1)*(1-(double)(pre_chan_2-n)/(pre_chan_2-pre_chan_1));
			}
			else
				gys=pre_chan_f1+(pre_chan_f2-pre_chan_f1)*(1-(double)(pre_chan_2-n)/(pre_chan_2-pre_chan_1));

		if ((n<=pre_chan_3) and (n>pre_chan_2))
			if (pre_chan_pb==1)
			{
			
			p_yn=pre_chan_pn2+(pre_chan_pn3-pre_chan_pn2)*(1-(double)(pre_chan_3-n)/(pre_chan_3-pre_chan_2));
			p_yp=pre_chan_pp2+(pre_chan_pp3-pre_chan_pp2)*(1-(double)(pre_chan_3-n)/(pre_chan_3-pre_chan_2));
			}
			else
				gys=pre_chan_f2+(pre_chan_f3-pre_chan_f2)*(1-(double)(pre_chan_3-n)/(pre_chan_3-pre_chan_2));

		if ((n<=pre_chan_4) and (n>pre_chan_3))
			if (pre_chan_pb==1)
			{
			p_yn=pre_chan_pn3+(pre_chan_pn4-pre_chan_pn3)*(1-(double)(pre_chan_4-n)/(pre_chan_4-pre_chan_3));
			p_yp=pre_chan_pp3+(pre_chan_pp4-pre_chan_pp3)*(1-(double)(pre_chan_4-n)/(pre_chan_4-pre_chan_3));
			}
			else
				gys=pre_chan_f3+(pre_chan_f4-pre_chan_f3)*(1-(double)(pre_chan_4-n)/(pre_chan_4-pre_chan_3));

		if ((n<=pre_chan_5) and (n>pre_chan_3))
			if (pre_chan_pb==1)
			{
			p_yn=pre_chan_pn4+(pre_chan_pn5-pre_chan_pn4)*(1-(double)(pre_chan_5-n)/(pre_chan_5-pre_chan_4));
			p_yp=pre_chan_pp4+(pre_chan_pp5-pre_chan_pp4)*(1-(double)(pre_chan_5-n)/(pre_chan_5-pre_chan_4));
			}
			else
				gys=pre_chan_f4+(pre_chan_f5-pre_chan_f4)*(1-(double)(pre_chan_5-n)/(pre_chan_5-pre_chan_4));


	 
		}


if (pressure_change==3)
		{
		
		if (n<=pre_chan_1)
			if (pre_chan_pb==1)
			{
			p_zn=p_zn_ori+(pre_chan_pn1-p_zn_ori)*(1-(double)(pre_chan_1-n)/pre_chan_1);
			p_zp=p_zp_ori+(pre_chan_pp1-p_zp_ori)*(1-(double)(pre_chan_1-n)/pre_chan_1);
			}
			else
				gzs=gx+(pre_chan_f1-gz)*(1-(double)(pre_chan_1-n)/pre_chan_1);

		if ((n<=pre_chan_2) and (n>pre_chan_1))
			if (pre_chan_pb==1)
			{
			
			p_zn=pre_chan_pn1+(pre_chan_pn2-pre_chan_pn1)*(1-(double)(pre_chan_2-n)/(pre_chan_2-pre_chan_1));
			p_zp=pre_chan_pp1+(pre_chan_pp2-pre_chan_pp1)*(1-(double)(pre_chan_2-n)/(pre_chan_2-pre_chan_1));
			}
			else
				gzs=pre_chan_f1+(pre_chan_f2-pre_chan_f1)*(1-(double)(pre_chan_2-n)/(pre_chan_2-pre_chan_1));

		if ((n<=pre_chan_3) and (n>pre_chan_2))
			if (pre_chan_pb==1)
			{
			
			p_zn=pre_chan_pn2+(pre_chan_pn3-pre_chan_pn2)*(1-(double)(pre_chan_3-n)/(pre_chan_3-pre_chan_2));
			p_zp=pre_chan_pp2+(pre_chan_pp3-pre_chan_pp2)*(1-(double)(pre_chan_3-n)/(pre_chan_3-pre_chan_2));
			}
			else
				gzs=pre_chan_f2+(pre_chan_f3-pre_chan_f2)*(1-(double)(pre_chan_3-n)/(pre_chan_3-pre_chan_2));

		if ((n<=pre_chan_4) and (n>pre_chan_3))
			if (pre_chan_pb==1)
			{
			p_zn=pre_chan_pn3+(pre_chan_pn4-pre_chan_pn3)*(1-(double)(pre_chan_4-n)/(pre_chan_4-pre_chan_3));
			p_zp=pre_chan_pp3+(pre_chan_pp4-pre_chan_pp3)*(1-(double)(pre_chan_4-n)/(pre_chan_4-pre_chan_3));
			}
			else
				gzs=pre_chan_f3+(pre_chan_f4-pre_chan_f3)*(1-(double)(pre_chan_4-n)/(pre_chan_4-pre_chan_3));

		if ((n<=pre_chan_5) and (n>pre_chan_3))
			if (pre_chan_pb==1)
			{
			p_zn=pre_chan_pn4+(pre_chan_pn5-pre_chan_pn4)*(1-(double)(pre_chan_5-n)/(pre_chan_5-pre_chan_4));
			p_zp=pre_chan_pp4+(pre_chan_pp5-pre_chan_pp4)*(1-(double)(pre_chan_5-n)/(pre_chan_5-pre_chan_4));
			}
			else
				gzs=pre_chan_f4+(pre_chan_f5-pre_chan_f4)*(1-(double)(pre_chan_5-n)/(pre_chan_5-pre_chan_4));






		}

}


void pressure_capillary()
{

	int rank = MPI :: COMM_WORLD . Get_rank ();
	int mpi_size=MPI :: COMM_WORLD . Get_size ();

	double error_sat_d;
	
	
	
//	if (n%freRe==0)
//	        cout<<n<<"         ffffff"<<endl;
	
	
        error_sat_d=abs((S_l_r-S_l)/S_l);
        if ((error_sat_d<sat_cri_d) and (n%freRe==0) and (n>start_pre_n+interval_pre) and (num_interval<chan_no))
        {
                if (ind_error_sat==1)
                error_sat=error_sat+1;
                else
                {
                error_sat=0;
                ind_error_sat=1;
                }
                       
        }
        else
              if ((error_sat_d>=sat_cri_d) and (n%freRe==0) and (n>start_pre_n+interval_pre) and (num_interval<chan_no))  
              {
                      ind_error_sat=0;
                      error_sat=0;
              }
        
        
       // if (n%freRe==0)
          //      cout<<n<<"         bbbbb"<<"   "<<rank<<endl;
        //cout<<n<<"   "<<start_pre_n<<endl;
        
        if ((n<=start_pre_n+interval_pre) and (num_interval<chan_no))
        { 
                
                if (pre_chan_pb2==1)
                {
                        if (pressure_change2==1)
                        {
                        p_xn=p_xn_ori+pre_in*(1-(double)(start_pre_n+interval_pre-n)/interval_pre);
                        p_xp=p_xp_ori+pre_ip*(1-(double)(start_pre_n+interval_pre-n)/interval_pre);
                        }
                
                        if (pressure_change2==2)
                        {
                        p_yn=p_yn_ori+pre_in*(1-(double)(start_pre_n+interval_pre-n)/interval_pre);
                        p_yp=p_yp_ori+pre_ip*(1-(double)(start_pre_n+interval_pre-n)/interval_pre);
                        }
                
                        if (pressure_change2==3)
                        {
                        p_zn=p_zn_ori+pre_in*(1-(double)(start_pre_n+interval_pre-n)/interval_pre);
                        p_zp=p_zp_ori+pre_ip*(1-(double)(start_pre_n+interval_pre-n)/interval_pre);
                        }
                }
                else
                {
                         if (pressure_change2==1)
                         gxs=gx+pre_in*(1-(double)(start_pre_n+interval_pre-n)/interval_pre);
            
                
               if (pressure_change2==2)
                        gys=gy+pre_in*(1-(double)(start_pre_n+interval_pre-n)/interval_pre);
                        
                
                if (pressure_change2==3)
                        gzs=gz+pre_in*(1-(double)(start_pre_n+interval_pre-n)/interval_pre);
                       
                        
                }
                
                
                       
        }
        
        
        
        
         if (error_sat>=sat_cri)
                                {
                                error_sat=0;
                                ind_error_sat=0;
                               // cout<<n<<"      ssssssssssss"<<endl;
                                start_pre_n=n;
                                num_interval++;
                                
                                if (pre_chan_pb2==1)
                                {
                                if (pressure_change2==1)
                                {
                                                p_xn_ori=p_xn;
                                                p_xp_ori=p_xp;
                                                if (rank==0)
                                                {
                                                 ofstream finfcp(FileName6,ios::app);
                                                 finfcp<<S_l<<" "<<1-S_l<<" "<<abs(p_xp-p_xn)/3<<endl;
                                                 finfcp.close();
                                                }
                                                
                                        }
                                        
                                 if (pressure_change2==2)
                                        {
                                                p_yn_ori=p_yn;
                                                p_yp_ori=p_yp;
                                                if (rank==0)
                                                {
                                                ofstream finfcp(FileName6,ios::app);
                                                 finfcp<<S_l<<" "<<1-S_l<<" "<<abs(p_yn-p_yp)/3<<endl;
                                                 finfcp.close();
                                                }
                                                
                                        }     
                                        
                                if (pressure_change2==3)
                                        {
                                                p_zn_ori=p_zn;
                                                p_zp_ori=p_zp;
                                                if (rank==0)
                                                {
                                                ofstream finfcp(FileName6,ios::app);
                                                 finfcp<<S_l<<" "<<1-S_l<<" "<<abs(p_zn-p_zp)/3<<endl;
                                                 finfcp.close();
                                                }
                                                
                                                
                                        }
                                }
                                else
                                        {
                                         if (pressure_change2==1)
                                         {
                                                gx=gxs;
                                                if (rank==0)
                                                {
                                                 ofstream finfcp(FileName6,ios::app);
                                                 //finfcp<<S_l<<" "<<1-S_l<<" "<<gxs*(per_xp-per_xn+1)<<endl;
						 finfcp<<S_l<<" "<<1-S_l<<" "<<gxs*(NX+1)<<endl;
                                                 finfcp.close();
                                                }
                                                
                                        }
                                        
                                 if (pressure_change2==2)
                                        {
                                                gy=gys;
                                               
                                                if (rank==0)
                                                {
                                                ofstream finfcp(FileName6,ios::app);
                                                 //finfcp<<S_l<<" "<<1-S_l<<" "<<gys*(per_yp-per_yn+1)<<endl;
						finfcp<<S_l<<" "<<1-S_l<<" "<<gys*(NY+1)<<endl;
                                                 finfcp.close();
                                                }
                                                
                                        }     
                                        
                                if (pressure_change2==3)
                                        {
                                                gz=gzs;
                                                if (rank==0)
                                                {
                                                ofstream finfcp(FileName6,ios::app);
                                                 //finfcp<<S_l<<" "<<1-S_l<<" "<<gzs*(per_zp-per_zn+1)<<endl;
						finfcp<<S_l<<" "<<1-S_l<<" "<<gzs*(NZ+1)<<endl;
                                                 finfcp.close();
                                                }
                                                
                                                
                                        }
                                        
                                        
                                        
                                        
                                        
                                        
                                        
                                        
                                                                                                
                                        
                                        }
                          
                                        
                                      
                                  
                                }        
                                
                                   
                                   
                                 
}
    


int inverse(mat &a){
    double t;
    int i,j,k,is[MAXN],js[MAXN];
    if(a.n!=a.m) return 0;
    for(k=0;k<a.n;k++){
        for(t=0,i=k;i<a.n;i++)
            for(j=k;j<a.n;j++)
                if(fabs(a.data[i][j])>t)
                    t=fabs(a.data[is[k]=i][js[k]=j]);
        if(zero(t)) return 0;
        if(is[k]!=k)
            for(j=0;j<a.n;j++)
                t=a.data[k][j],a.data[k][j]=a.data[is[k]][j],a.data[is[k]][j]=t;
        if(js[k]!=k)
            for(i=0;i<a.n;i++)
                t=a.data[i][k],a.data[i][k]=a.data[i][js[k]],a.data[i][js[k]]=t;
        a.data[k][k]=1/a.data[k][k];
        for(j=0;j<a.n;j++)
            if(j!=k)
                a.data[k][j]*=a.data[k][k];
        for(i=0;i<a.n;i++)
            if(i!=k)
                for(j=0;j<a.n;j++)
                    if(j!=k)
                        a.data[i][j]-=a.data[i][k]*a.data[k][j];
        for(i=0;i<a.n;i++)
            if(i!=k)
                a.data[i][k]*=-a.data[k][k];
    }
    for(k=a.n-1;k>=0;k--){
        for(j=0;j<a.n;j++)
            if(js[k]!=k)
                t=a.data[k][j],a.data[k][j]=a.data[js[k]][j],a.data[js[k]][j]=t;
        for(i=0;i<a.n;i++)
            if(is[k]!=k)
                t=a.data[i][k],a.data[i][k]=a.data[i][is[k]],a.data[i][is[k]]=t;
    }
    return 1;
}


void Comput_MI(double M[19][19], double MI[19][19])
{

double mim[19][19];

mat a;
    int i,j;
    int n_s=19;
        for(int i=0;i<n_s;i++)
            for(int j=0;j<n_s;j++)
                a.data[i][j]=M[i][j];
	a.m=a.n=n_s;
        if(inverse(a))
            for(int i=0;i<n_s;i++)
                for(int j=0;j<n_s;j++)
                    MI[i][j]=a.data[i][j];
               
            
        else
            puts("NO");


}




void Parallelize_Geometry()
{

   int rank = MPI :: COMM_WORLD . Get_rank ();
    int mpi_size=MPI :: COMM_WORLD . Get_size ();
    int nx=NX+1;
    int ny=NY+1;
    int nz=NZ+1;
    
    int ii,jj,kk;
    int ip,jp,kp;
    
   if (par_per_x==0)
		{per_xn=0;per_xp=NX;}
	if (par_per_y==0)
		{per_yn=0;per_yp=NY;}
	if (par_per_z==0)
		{per_zn=0;per_zp=NZ;}
                                
    int procind=MPI :: COMM_WORLD . Get_rank ()+1;                               //currentprocessor index, start from 1
    int procn=MPI :: COMM_WORLD . Get_size ();                                  //total processor number
    int neib=0;
   
            bufinfo=new int[procn+1];
            for (int i=0;i<=procn;i++)
                    bufinfo[i]=0;   
            
            
    com_n=0;                                                                    //mpi commu numbers     number of neighbour partitions which need communication
    int tmpint;
    int proc_com[procn+1];                                              //index convert proc index---->commu index in current processor
    for (int i=0;i<=procn;i++)
        proc_com[i]=0;
        
      //-------------------
      int* sumtmp;
      //-------------------  
        
   //cout<<"@@@@@@@@@@@@@@@@@@@@@@@@"<<endl;
	
	Solid = new int**[nx];
      Solid2 = new int**[nx];
	
	
	
	for (int i=0;i<nx;i++)				///*********
		Solid[i]=new int*[ny],Solid2[i]=new int*[ny];

	Solid[0][0]=new int[nx*ny*nz],Solid2[0][0]=new int[nx*ny*nz];

	
 	for (int i=1;i<ny;i++)
               Solid[0][i]=Solid[0][i-1]+nz,Solid2[0][i]=Solid2[0][i-1]+nz;
       
       for (int i=1;i<nx;i++)
       {
               Solid[i][0]=Solid[i-1][0]+ny*nz,Solid2[i][0]=Solid2[i-1][0]+ny*nz;
               for (int j=1;j<ny;j++)
                       Solid[i][j]=Solid[i][j-1]+nz,Solid2[i][j]=Solid2[i][j-1]+nz;
       }	
	
      
      for(int k=0 ; k<=NZ ; k++)
	for(int j=0 ; j<=NY ; j++)
	for(int i=0 ; i<=NX ; i++)
		Solid[i][j][k]=0,Solid2[i][j][k]=0;


	
	//MPI_Barrier(MPI_COMM_WORLD);
      //cout<<"@@@@@@@@@@@@@   "<<rank<<endl;
      
      
   porosity=0.0;
   
 
    int* recv_solid;
    int pore;
   
      if (rank==0)
{	

	fstream fin;
	if (decbin==0)
	{
	FILE *ftest;
	//ifstream fin;
	
	ftest = fopen(filename, "r");

	if(ftest == NULL)
	{
		cout << "\n The pore geometry file (" << filename <<
			") does not exist!!!!\n";
		cout << " Please check the file\n\n";

		exit(-1);
	}
	fclose(ftest);
	
	fin.open(filename);
	for(int k=0 ; k<=NZ ; k++)
	for(int j=0 ; j<=NY ; j++)
	for(int i=0 ; i<=NX ; i++)
	
	{
		
			fin >> pore;
			
	
		
		Solid[i][j][k]=pore;
		
	}
	fin.close();

	}
	else
	{
	//fstream fin;
	fin.open(filename,ios::in);
	if (fin.fail())
	        {
	        cout<<"\n file open error on " << filename<<endl;
	        exit(-1);
	        }
	//cout<<filename<<endl;
	fin.read((char *)(Solid[0][0]), sizeof(int)*(NX+1)*(NY+1)*(NZ+1));
	
	fin.close();
	}
	
	

}
  	//MPI_Barrier(MPI_COMM_WORLD);
	//cout<<"##########   "<<rank<<endl;
        MPI_Bcast(Solid[0][0],(NX+1)*(NY+1)*(NZ+1),MPI_INT,0,MPI_COMM_WORLD);
	

        for (int k=0;k<=NZ;k++)
		for (int j=0;j<=NY;j++)
        		for (int i=0;i<=NX;i++)
                
			{
	                //Solid[i][j][k]=Solid_rank0[i*(NY+1)*(NZ+1)+j*(NZ+1)+k];
			if ((Solid[i][j][k]==0) and (i>=per_xn) and (i<=per_xp) and (j>=per_yn) and (j<=per_yp) and (k>=per_zn) and (k<=per_zp)) 
			        porosity+=1.0;
			}
	        sumss=new int [procn+1];
	for (int i=0;i<=procn;i++)
	        sumss[i]=0;
	



	porosity=porosity/(double)((per_xp-per_xn+1)*(per_yp-per_yn+1)*(per_zp-per_zn+1));

	Partition_Solid(Solid);

	//cout<<porosity<<endl;
	
	for(int k=0 ; k<nz ; k++)			
	for(int j=0 ; j<ny ; j++)
	for(int i=0 ; i<nx ; i++)			
	        if (Solid[i][j][k]>0)
	        {
	                sumss[Solid[i][j][k]]++;
	                Solid2[i][j][k]=sumss[Solid[i][j][k]];
	                
	
	
	                //=======calculate neibough numbers==========
	                if (Solid[i][j][k]==procind)
	                for (int ls=1;ls<19;ls++)
	                {
	                        ii=i+e[ls][0];
	                                if (ii<0)
	                                        ii=nx-1;
	                                if (ii>=nx)
	                                        ii=0;
	                                
	                                        
	                        jj=j+e[ls][1];
	                                if (jj<0)
	                                        jj=ny-1;
	                                if (jj>=ny)
	                                        jj=0;
	                                
	                        kk=k+e[ls][2];
	                                       if (kk<0)
	                                        kk=nz-1;
	                                        if (kk>=nz)
	                                        kk=0; 
	                        
	                                if ((Solid[ii][jj][kk]>0) and (Solid[ii][jj][kk]!=procind))
	                                        {
	                                                bufinfo[Solid[ii][jj][kk]]++;
	                                        //cout<<procind<<"        "<<Solid[ii][jj][kk]<<endl;
	                                        }
	                                
	                                        
	                
	                }
	        //=======================================================
	        }
	        else
	                Solid2[i][j][k]=0;


	
	       
	        //======coordinate of nodes===========
	        coor = new int[sumss[procind]+1];
	                for(int k=0 ; k<nz ; k++)			
	                for(int j=0 ; j<ny ; j++)
	                for(int i=0 ; i<nx ; i++)
	                        if (Solid[i][j][k]==procind)
	                                coor[Solid2[i][j][k]]=i*ny*nz+j*nz+k;
	        //=============================
	        
	        
	        for (int i=1;i<=procn;i++)
	        {
	                //cout<<bufinfo[i]<<endl;
	                if (bufinfo[i]>0)
	                        com_n++;
	        }
	        com_ind=new int[com_n];
	                
	        tmpint=0;
	        for (int i=1;i<=procn;i++)
	                if (bufinfo[i]>0)
	                {com_ind[tmpint]=i;proc_com[i]=tmpint;tmpint++;}
	                
	
	
	bufsend = new double* [com_n];
	        for (int i=0;i<com_n;i++)
	                bufsend[i] = new double[bufinfo[com_ind[i]]];
	        
	bufrecv = new double* [com_n];
	        for (int i=0;i<com_n;i++)
	                bufrecv[i] = new double[bufinfo[com_ind[i]]];
	        
	        
	 bufsendrhor = new double* [com_n];
	        for (int i=0;i<com_n;i++)
	                bufsendrhor[i] = new double[bufinfo[com_ind[i]]];
	        
	bufrecvrhor = new double* [com_n];
	        for (int i=0;i<com_n;i++)
	                bufrecvrhor[i] = new double[bufinfo[com_ind[i]]];
	        
	bufsendrhob = new double* [com_n];
	        for (int i=0;i<com_n;i++)
	                bufsendrhob[i] = new double[bufinfo[com_ind[i]]];
	        
	bufrecvrhob = new double* [com_n];
	        for (int i=0;i<com_n;i++)
	                bufrecvrhob[i] = new double[bufinfo[com_ind[i]]];        
	        
	        
	        
	nei_loc= new int*[sumss[procind]+1];
	nei_loc[0] =new int[(sumss[procind]+1)*19];
	        for (int i=1;i<=sumss[procind];i++)
	                nei_loc[i] = nei_loc[i-1]+19;
	  
	        buflocsend = new int*[com_n];
	        for (int i=0;i<com_n;i++)
	                buflocsend[i] = new int[bufinfo[com_ind[i]]];
	        
	        buflocrecv = new int*[com_n];
	        for (int i=0;i<com_n;i++)
	                buflocrecv[i] = new int[bufinfo[com_ind[i]]];
	        
	        sumtmp = new int[com_n];
	                for(int i=0;i<com_n;i++)
	                        sumtmp[i]=0;
	                
	                
	
	        for (int ci=1;ci<=sumss[procind];ci++)
	                {
	                      ii=(int)(coor[ci]/(ny*nz));
	                      jj=(int)((coor[ci]%(ny*nz))/nz);
	                      kk=(int)(coor[ci]%nz);
	                      
	                      for (int mi=0; mi<19; mi++)
			{
			
			
			        ip=ii+e[mi][0];if (ip<0) {ip=nx-1;};if (ip>=nx) {ip=0;};
			        jp=jj+e[mi][1];if (jp<0) {jp=ny-1;}; if (jp>=ny) {jp=0;};
			        kp=kk+e[mi][2];if (kp<0) {kp=nz-1;}; if (kp>=nz) {kp=0;};
			        
			        if (Solid[ip][jp][kp]==procind)
			                nei_loc[ci][mi]=Solid2[ip][jp][kp];
			        else
			                if (Solid[ip][jp][kp]==0)
			                        nei_loc[ci][mi]=0;
			                else
			                {
			                       // nei_loc[ci][mi]=-Solid[ip][jp][kp];
			                       nei_loc[ci][mi]=-proc_com[Solid[ip][jp][kp]]-1;
			                        buflocsend[proc_com[Solid[ip][jp][kp]]][sumtmp[proc_com[Solid[ip][jp][kp]]]]=Solid2[ip][jp][kp]*19+mi;
			                      
			                        sumtmp[proc_com[Solid[ip][jp][kp]]]++;
			                       
			                }
			                
			}
			                
			                
	                    
	                }
	                
	                
	     
	                                               
	       MPI_Status status[com_n*2] ;
	       MPI_Request request[com_n*2];         
	       int mpi_test=procn;
	       
	       for (int i=0;i<com_n;i++)
	       
	               {
	       MPI_Isend(buflocsend[i],bufinfo[com_ind[i]], MPI_INT, com_ind[i]-1, (procind-1)*procn+com_ind[i]-1, MPI_COMM_WORLD,&request[2*i]);
	                       
	       MPI_Irecv(buflocrecv[i],bufinfo[com_ind[i]], MPI_INT, com_ind[i]-1, (com_ind[i]-1)*procn+procind-1, MPI_COMM_WORLD,&request[2*i+1]);		
	               }
	               
	               
	               
	      //-------------BCs--------------------------------------------------------         
	               bclxn=0;bclyn=0;bclzn=0;bcrxn=0;bcryn=0;bcrzn=0;
	                
	                 for(int k=0; k<nz ; k++)			
	                 for(int j=0 ; j<ny ; j++)
	                 {
	                         if (Solid[0][j][k]==procind)
	                                 bclxn++;
	                         if (Solid[nx-1][j][k]==procind)
	                                 bcrxn++;
	                 }
	                 if (bclxn>0)
	                         bclx=new int[bclxn];
	                 if (bcrxn>0)
	                         bcrx= new int[bcrxn];
	                 bclxn=0;bcrxn=0;
	                 for(int k=0; k<nz ; k++)			
	                 for(int j=0 ; j<ny ; j++)
	                 {
	                         if (Solid[0][j][k]==procind)
	                                 bclx[bclxn]=Solid2[0][j][k],bclxn++;
	                         
	                         if (Solid[nx-1][j][k]==procind)
	                                 bcrx[bcrxn]=Solid2[nx-1][j][k],bcrxn++;
	                 }
	                 
	                 
	                 
	               for(int k=0 ; k<nz ; k++)			
	               for(int i=0 ; i<nx ; i++)  
	                 {
	                         if (Solid[i][0][k]==procind)
	                                 bclyn++;
	                         if (Solid[i][ny-1][k]==procind)
	                                 bcryn++;
	                 }
	                 if (bclyn>0)
	                         bcly=new int[bclyn];
	                 if (bcryn>0)
	                         bcry= new int[bcryn];
	                 bclyn=0;bcryn=0;
	                for(int k=0 ; k<nz ; k++)			
	                        for(int i=0 ; i<nx ; i++)  
	                 {
	                         if (Solid[i][0][k]==procind)
	                                 bcly[bclyn]=Solid2[i][0][k],bclyn++;
	                         
	                         if (Solid[i][ny-1][k]==procind)
	                                 bcry[bcryn]=Solid2[i][ny-1][k],bcryn++;
	                 }   
	                 
	                 
	                 for(int j=0 ; j<ny ; j++)     
	                 for(int i=0 ; i<nx ; i++)  
	                 {
	                         if (Solid[i][j][0]==procind)
	                                 bclzn++;
	                         if (Solid[i][j][nz-1]==procind)
	                                 bcrzn++;
	                 }
	                 if (bclzn>0)
	                         bclz=new int[bclzn];
	                 if (bcrzn>0)
	                         bcrz= new int[bcrzn];
	                 bclzn=0;bcrzn=0;
	               for(int j=0 ; j<ny ; j++)     
	                 for(int i=0 ; i<nx ; i++) 
	                 {
	                         if (Solid[i][j][0]==procind)
	                                 bclz[bclzn]=Solid2[i][j][0],bclzn++;
	                         
	                         if (Solid[i][j][nz-1]==procind)
	                                 bcrz[bcrzn]=Solid2[i][j][nz-1],bcrzn++;
	                 }   
	                 
	      //---------------------------------------------------------------------------           
	                      
	                 
      		MPI_Waitall(2*com_n,request, status);
      		MPI_Testall(2*com_n,request,&mpi_test,status);
	      Count=sumss[procind];  

cout<<"GEOMETRY FILE PARTITIONING FOR PARALLEL READING DONE   Processor No."<<rank<<endl;


float* Psi_rank0;	      
Psi_rank0 = new float[(NX+1)*(NY+1)*(NZ+1)];
if (mix_psi_thickness>0)
        psi_mixture_ini(Psi_rank0);
	      

Psi_local = new float[Count+1];

float pore2;
 

 if (rank==0)
         if ((ini_Sat<0) and (mix_psi_thickness==0))
{       
        FILE *ftest;
	ftest = fopen(filenamepsi, "r");
	ifstream fin;
	if(ftest == NULL)
	{
		cout << "\n The Concentration file (" << filenamepsi <<
			") does not exist!!!!\n";
		cout << " Please check the file\n\n";

		exit(0);
	}
	fclose(ftest);

	
	
	fin.open(filenamepsi);
	for(int k=0 ; k<=NZ ; k++)
	for(int j=0 ; j<=NY ; j++)
	for(int i=0 ; i<=NX ; i++)
	
	{
	
			fin >> pore2;
			Psi_rank0[i*(NY+1)*(NZ+1)+j*(NZ+1)+k]=pore2;
	
	
	}
	fin.close();
	
}


        MPI_Bcast(Psi_rank0,(NX+1)*(NY+1)*(NZ+1),MPI_FLOAT,0,MPI_COMM_WORLD);
	tmpint=1;
        for(int k=0 ; k<=NZ ; k++)
	for(int j=0 ; j<=NY ; j++)
	for(int i=0 ; i<=NX ; i++)
	        if (Solid[i][j][k]==procind)
	                Psi_local[tmpint]=Psi_rank0[i*ny*nz+j*nz+k],tmpint++;
	        
 
	        
	 cout<<"CONCENTRATION FILE PARTITIONING FOR PARALLEL READING DONE   Processor No."<<rank<<endl;

   
	
	 
	 
	  delete [] Psi_rank0;
	  
	   for (int i=0;i<com_n;i++)
	           delete [] buflocsend[i];
	   delete [] buflocsend;
	   
	   delete [] sumtmp;
	   
	   delete [] Solid2[0][0];
		for (int i=0;i<nx;i++)
			delete [] Solid2[i];
		delete [] Solid2;	
	 
	   if (rank>0)
	   {
	      
	           delete [] Solid[0][0];
			for (int i=0;i<nx;i++)
			delete [] Solid[i];
		delete [] Solid;
	   }
	 
	 
	 
	 
}





void init(double* rho, double** u, double** f,double* psi,double* rho_r, double* rho_b, double* rhor, double* rhob, int* SupInv)
{	
       int rank = MPI :: COMM_WORLD . Get_rank ();
	int mpi_size=MPI :: COMM_WORLD . Get_size ();

	
	int nx_g[mpi_size];
	int disp[mpi_size];
	int si,sj,sm;
	
	MPI_Gather(&nx_l,1,MPI_INT,nx_g,1,MPI_INT,0,MPI_COMM_WORLD);
	
	
	if (rank==0)
		{
		disp[0]=0;
	
		for (int i=1;i<mpi_size;i++)
			disp[i]=disp[i-1]+nx_g[i-1];
		
		}

	MPI_Bcast(disp,mpi_size,MPI_INT,0,MPI_COMM_WORLD);

        srand((unsigned)time(0)+rank);
	
	double usqr,vsqr,rand_double;
	double c2,c4,sat_tmp;
	
	rho0=1.0;dt=1.0/Zoom;dx=1.0/Zoom;
 
	if (lattice_v==1)
		{dx=dx_input;dt=dt_input;}

	lat_c=dx/dt;
	c_s=lat_c/sqrt(3);
	c_s2=lat_c*lat_c/3;

	c2=lat_c*lat_c;c4=c2*c2;
	
	
	niu=in_vis;
	
	//tau_f=3.0*niu/dt+0.5;
	tau_f=niu/(c_s2*dt)+0.5;
	s_v=1/tau_f;
        
	//double pr; //raduis of the obstacles
       int loc_x,loc_y,loc_z;
       
	double s_other=8*(2-s_v)/(8-s_v);
	double u_tmp[3];



	//=====VISCOSITY INTERPOLATION======================
	delta=0.1;
	wl=1.0/(niu_l/(c_s2*dt)+0.5);
	wg=1.0/(niu_g/(c_s2*dt)+0.5);
	lg0=2*wl*wg/(wl+wg);
	l1=2*(wl-lg0)/delta;
	l2=-l1/(2*delta);
	g1=2*(lg0-wg)/delta;
	g2=g1/(2*delta);
	//=================================================



	S[0]=0;
	S[1]=s_v;
	S[2]=s_v;
	S[3]=0;
	S[4]=s_other;
	S[5]=0;
	S[6]=s_other;
	S[7]=0;
	S[8]=s_other;
	S[9]=s_v;
	S[10]=s_v;
	S[11]=s_v;
	S[12]=s_v;
	S[13]=s_v;
	S[14]=s_v;
	S[15]=s_v;
	S[16]=s_other;
	S[17]=s_other;
	S[18]=s_other;


	if (lattice_v==1)
	{
	
	M_c[0]=1.0;
	M_c[1]=lat_c*lat_c;
	M_c[2]=lat_c*lat_c*lat_c*lat_c;
	M_c[3]=lat_c;
	M_c[4]=lat_c*lat_c*lat_c;
	M_c[5]=lat_c;
	M_c[6]=lat_c*lat_c*lat_c;
	M_c[7]=lat_c;	
	M_c[8]=lat_c*lat_c*lat_c;
	M_c[9]=lat_c*lat_c;
	M_c[10]=lat_c*lat_c*lat_c*lat_c;
	M_c[11]=lat_c*lat_c;
	M_c[12]=lat_c*lat_c*lat_c*lat_c;
	M_c[13]=lat_c*lat_c;
	M_c[14]=lat_c*lat_c;
	M_c[15]=lat_c*lat_c;
	M_c[16]=lat_c*lat_c*lat_c;
	M_c[17]=lat_c*lat_c*lat_c;
	M_c[18]=lat_c*lat_c*lat_c;



	for (int i=0;i<19;i++)
		for (int j=0;j<3;j++)
		elat[i][j]=e[i][j]*lat_c;

	for (int i=0;i<19;i++)
		for (int j=0;j<19;j++)
		M[i][j]*=M_c[i];

	Comput_MI(M,MI);

	}
       

	
//=============Rel_Perm_Imb_Drai======================

	
	if (rel_perm_id_dir>0)
	{
	        
	rel_perm_id_ids=0;
	rel_perm_id_mode=1;
	in_psi_BC=1;
	
	
	
	}
//==================================================





	psi_solid=ContactAngle_parameter;

	if (ini_Sat<0)
	for (int i=1;i<=Count;i++)	
			
		{
			u[i][0]=inivx;
			u[i][1]=inivy;
			u[i][2]=inivz;
			u_tmp[0]=u[i][0];
			u_tmp[1]=u[i][1];
			u_tmp[2]=u[i][2];
			psi[i]=Psi_local[i];
			rho[i]=1.0;
			rho_r[i]=(psi[i]*rho[i]+rho[i])/2;
			rho_b[i]=rho[i]-rho_r[i];
			rhor[i]=0;
			rhob[i]=0;
			

			//forcex[i]=gx;
			//forcey[i]=gy;
			//forcez[i]=gz;
			if (stab==1)
				{gxs=0;gys=0;gzs=0;}
			else
				{gxs=gx;gys=gy;gzs=gz;}

			

			//INITIALIZATION OF m and f

			for (int lm=0;lm<19;lm++)
					f[i][lm]=feq(lm,rho[i],u_tmp);	

	}
	else
	for (int i=1;i<=Count;i++)	
			
		{
			u[i][0]=inivx;
			u[i][1]=inivy;
			u[i][2]=inivz;
			u_tmp[0]=u[i][0];
			u_tmp[1]=u[i][1];
			u_tmp[2]=u[i][2];
			rand_double=(double(rand()%10000))/10000;
			if (rand_double<ini_Sat)
			        psi[i]=1;
			else
			        psi[i]=-1;
			
			Psi_local[i]=psi[i];
			rho[i]=1.0;
			rho_r[i]=(psi[i]*rho[i]+rho[i])/2;
			rho_b[i]=rho[i]-rho_r[i];
			rhor[i]=0;
			rhob[i]=0;
			

			
			if (stab==1)
				{gxs=0;gys=0;gzs=0;}
			else
				{gxs=gx;gys=gy;gzs=gz;}

			

			//INITIALIZATION OF m and f

			for (int lm=0;lm<19;lm++)
			
					f[i][lm]=feq(lm,rho[i],u_tmp);	
			
			if ((par_per_x>0) or (par_per_y>0) or (par_per_z>0))
			        {        
			                loc_x=(int)(coor[i]/((NY+1)*(NZ+1)));
			                loc_y=(int)((coor[i]%((NY+1)*(NZ+1)))/(NZ+1));
			                loc_z=coor[i]%(NZ+1);
			                
			                //if ((loc_y<per_yn) and (loc_y>per_yp) and (par_per_y>0))
			                 //cout<<loc_x<<"        "<<loc_y<<"        "<<loc_z<<"        "<<par_per_x<<endl;
			                 //cout<<loc_x<<"        "<<loc_y<<"        "<<loc_z<<"        "<<per_yn<<"        "<<per_yp<<"        "<<ini_buf<<endl;
			                //cout<<per_xn<<endl;
			                
			                if ( (((loc_x<per_xn) or (loc_x>per_xp)) and (par_per_x>0)) or (((loc_y<per_yn) or (loc_y>per_yp)) and (par_per_y>0)) or (((loc_z<per_zn) or (loc_z>per_zp)) and (par_per_z>0)) )
			                       
			                 if ((ini_buf==-1) or (ini_buf==1))
			                {        
			                        //cout<<loc_x<<"        "<<loc_y<<"        "<<loc_z<<"        "<<par_per_x<<endl;
			                        psi[i]=ini_buf;
			                        rho_r[i]=(psi[i]*rho[i]+rho[i])/2;
			                        rho_b[i]=rho[i]-rho_r[i];
			                        Psi_local[i]=ini_buf;
			                       
			                }
			                
			                
			                
			        }
					
	}



       		//-------------------------------------
		S_l=Comput_Saturation(psi,Solid,SupInv);
		//---------------------------------------



	
//===================Rel_Perm_Imb_Drai======================
rel_perm_id_ids=0;
sat_tmp=Comput_Saturation(psi,Solid,SupInv);
if (rel_perm_id_dir==1)
{
        while (((rel_perm_id_ids)<rel_perm_chan_num) &&  (sat_tmp>rel_perm_sw[rel_perm_id_ids]))
                {
                        rel_perm_id_ids++;
                }
}
        
if (rel_perm_id_dir==2)
{
        while (((rel_perm_id_ids)<rel_perm_chan_num) &&  (sat_tmp<rel_perm_sw[rel_perm_id_ids]))
                {
                        rel_perm_id_ids++;
                }
}       

        
/*        
if ((rel_perm_bodyf_mode==1) and (rel_perm_id_dir>0))
                  {
                          if (PerDir==1)
                                  gxs=rel_perm_bodyf;
                          if (PerDir==2)
                                  gys=rel_perm_bodyf;
                          if (PerDir==3)
                                  gzs=rel_perm_bodyf;
                  }	
*/
//=====================================================
	 	
}


inline double feq(int k,double rho, double u[3])
{

	double ux,uy,uz;
	double eu,uv,feq;
        double c2,c4,ls;
	
	double rho_0=1.0;
	
	c2=lat_c*lat_c;c4=c2*c2;
	eu=(elat[k][0]*u[0]+elat[k][1]*u[1]+elat[k][2]*u[2]);
	uv=(u[0]*u[0]+u[1]*u[1]+u[2]*u[2]);// WITH FORCE TERM:GRAVITY IN X DIRECTION
	feq=w[k]*rho*(1.0+3.0*eu/c2+4.5*eu*eu/c4-1.5*uv/c2);

			ux=u[0];
			uy=u[1];
			uz=u[2];

	for(int s=0;s<19;s++)
		meq[s]=0;
			meq[0]=rho;meq[3]=rho_0*ux;meq[5]=rho_0*uy;meq[7]=rho_0*uz;
			meq[1]=rho_0*(ux*ux+uy*uy+uz*uz);
			meq[9]=rho_0*(2*ux*ux-uy*uy-uz*uz);
			meq[11]=rho_0*(uy*uy-uz*uz);
			meq[13]=rho_0*ux*uy;meq[14]=rho_0*uy*uz;
			meq[15]=rho_0*ux*uz;

	feq=0;
	for (int j=0;j<19;j++)
		feq+=MI[k][j]*meq[j];		





	return feq;

}





void collision(double* rho,double** u,double** f,double** F,double* psi, double* rho_r, double* rho_b, double* rhor, double* rhob, int* SupInv,int*** Solid,int* Sl, int* Sr)
{

        
        MPI_Status status[com_n*2] ;
	MPI_Request request[com_n*2];
	
	MPI_Status status2[com_n*6] ;
	MPI_Request request2[com_n*6];
	
	
	
	int mpi_test;
	int mpi_test2;
	
	int rank = MPI :: COMM_WORLD . Get_rank ();
	int mpi_size=MPI :: COMM_WORLD . Get_size ();
	int procn=mpi_size; 
	int procind=rank+1;
	
	
	int testl1,testl2;
	int sumtmp[com_n];
	int sumtmp2[com_n];
	int sumtmp3[com_n];
	for (int i=0;i<com_n;i++)
	        sumtmp[i]=0,sumtmp2[i]=0,sumtmp3[i]=0;

double C[3];
double g_r[19],g_b[19];
double rho_0=1.0;
double lm0,lm1,cc,sum,uu;
double ux,uy,uz,nx,ny,nz;
double usqr,vsqr,eu,ef,cospsi,s_other;
double F_hat[19],GuoF[19],f_eq[19],u_tmp[3];
double m_l[19],m_inv_l[19];
int i,j,m,ind_S;
int interi,interj,interm,ip,jp,kp;
double c2,c4;
double delta_rho=0.1;
const double c_l=lat_c;

for (int i=0;i<com_n;i++)
        for(int j=0;j<bufinfo[com_ind[i]];j++)
        {
                bufsendrhor[i][j]=0;
                bufrecvrhor[i][j]=0;
                bufsendrhob[i][j]=0;
                bufrecvrhob[i][j]=0;
        }
        



//===========test==pertubation==============
double cb[19]={-1.0/3.0,1.0/18.0,1.0/18.0,1.0/18.0,1.0/18.0,1.0/18.0,1.0/18.0,1.0/36.0,1.0/36.0,1.0/36.0,1.0/36.0,1.0/36.0,1.0/36.0,1.0/36.0,1.0/36.0,1.0/36.0,1.0/36.0,1.0/36.0,1.0/36.0};
//==========================================



	c2=lat_c*lat_c;c4=c2*c2;


	for (int i=0;i<com_n;i++)
	        for (int j=0;j<bufinfo[com_ind[i]];j++)
	        {  
	                testl1=(int)(buflocrecv[i][j]/19);
	                testl2=(int)(buflocrecv[i][j]%19);
	                bufsend[i][j]=psi[testl1];
	        }
	        
	for (int i=0;i<com_n;i++)
	{
	MPI_Isend(bufsend[i],bufinfo[com_ind[i]], MPI_DOUBLE, com_ind[i]-1, (procind-1)*procn+com_ind[i]-1, MPI_COMM_WORLD,&request[2*i]);
	                       
	MPI_Irecv(bufrecv[i],bufinfo[com_ind[i]], MPI_DOUBLE, com_ind[i]-1, (com_ind[i]-1)*procn+procind-1, MPI_COMM_WORLD,&request[2*i+1]);		
	}

	               MPI_Waitall(2*com_n,request, status);
	               MPI_Testall(2*com_n,request,&mpi_test,status);
	               
	               
	 //cout<<"@@@@@@@@@@@@@@@@@@@"<<endl;              

	for(int ci=1;ci<=Count;ci++)	
	

		{	
				
		       //cout<<"**     "<<rank<<"	"<<ci<<"	"<<Count+1<<endl;   
		        i=(int)(coor[ci]/((NY+1)*(NZ+1)));
		        j=(int)((coor[ci]%((NY+1)*(NZ+1)))/(NZ+1));
			m=(int)(coor[ci]%(NZ+1));   

			C[0]=0;C[1]=0;C[2]=0;ind_S=0;
	for (int tmpi=0;tmpi<19;tmpi++)
		{
		        //cout<<f[ci][tmpi]<<endl;
			//-------------------PERIODIC BOUNDARY CONDITION---------------------------
		interi=i+e[tmpi][0];
		interj=j+e[tmpi][0];
		interm=m+e[tmpi][0];
			
		if (in_psi_BC>0)
		{
		    
		        //i=(int)(coor[ci]/((NY+1)*(NZ+1)));
		        //j=(int)((coor[ci]%((NY+1)*(NZ+1)))/(NZ+1));
			//m=(int)(coor[ci]%(NZ+1));   
		        
		        if (nei_loc[ci][tmpi]>0)
			        	{
					C[0]+=3.0/(lat_c*lat_c*dt)*w[tmpi]*elat[tmpi][0]*psi[nei_loc[ci][tmpi]];
					C[1]+=3.0/(lat_c*lat_c*dt)*w[tmpi]*elat[tmpi][1]*psi[nei_loc[ci][tmpi]];
					C[2]+=3.0/(lat_c*lat_c*dt)*w[tmpi]*elat[tmpi][2]*psi[nei_loc[ci][tmpi]];
			        	}        
			        else
			                if (nei_loc[ci][tmpi]==0)
			        	{
                                        ind_S=1;
			               	C[0]+=3.0/(lat_c*lat_c*dt)*w[tmpi]*elat[tmpi][0]*psi_solid;
					C[1]+=3.0/(lat_c*lat_c*dt)*w[tmpi]*elat[tmpi][1]*psi_solid;
					C[2]+=3.0/(lat_c*lat_c*dt)*w[tmpi]*elat[tmpi][2]*psi_solid;
			        	}
			        	else
			        	        if (((psi_xn>0) and (interi<0)) or ((psi_xp>0) and (interi>NX)) or ((psi_yn>0) and (interj<0)) or ((psi_yp>0) and (interj>NY)) or ((psi_zn>0) and (interm<0)) or ((psi_zp>0) and (interm>NZ)))
			        	        {
			        	                //sumtmp[-nei_loc[ci][tmpi]]++;
			        	                C[0]+=3.0/(lat_c*lat_c*dt)*w[tmpi]*elat[tmpi][0]*psi[ci];
			        	                C[1]+=3.0/(lat_c*lat_c*dt)*w[tmpi]*elat[tmpi][1]*psi[ci];
			        	                C[2]+=3.0/(lat_c*lat_c*dt)*w[tmpi]*elat[tmpi][2]*psi[ci];       
			        	        }
			        	        else
			        	                {
	C[0]+=3.0/(lat_c*lat_c*dt)*w[tmpi]*elat[tmpi][0]*bufrecv[-nei_loc[ci][tmpi]-1][sumtmp[-nei_loc[ci][tmpi]-1]];
	C[1]+=3.0/(lat_c*lat_c*dt)*w[tmpi]*elat[tmpi][1]*bufrecv[-nei_loc[ci][tmpi]-1][sumtmp[-nei_loc[ci][tmpi]-1]];
	C[2]+=3.0/(lat_c*lat_c*dt)*w[tmpi]*elat[tmpi][2]*bufrecv[-nei_loc[ci][tmpi]-1][sumtmp[-nei_loc[ci][tmpi]-1]];      
			        	                   sumtmp[-nei_loc[ci][tmpi]-1]++;        
			        	                }
			        	                        
		}
		else
		        {
		        if (nei_loc[ci][tmpi]>0)
			        	{
					C[0]+=3.0/(lat_c*lat_c*dt)*w[tmpi]*elat[tmpi][0]*psi[nei_loc[ci][tmpi]];
					C[1]+=3.0/(lat_c*lat_c*dt)*w[tmpi]*elat[tmpi][1]*psi[nei_loc[ci][tmpi]];
					C[2]+=3.0/(lat_c*lat_c*dt)*w[tmpi]*elat[tmpi][2]*psi[nei_loc[ci][tmpi]];
			        	}        
			        else
			                if (nei_loc[ci][tmpi]==0)
			        	{
                                        ind_S=1;
			               	C[0]+=3.0/(lat_c*lat_c*dt)*w[tmpi]*elat[tmpi][0]*psi_solid;
					C[1]+=3.0/(lat_c*lat_c*dt)*w[tmpi]*elat[tmpi][1]*psi_solid;
					C[2]+=3.0/(lat_c*lat_c*dt)*w[tmpi]*elat[tmpi][2]*psi_solid;
			        	}
			        	else
			        	       
			        	                {
               C[0]+=3.0/(lat_c*lat_c*dt)*w[tmpi]*elat[tmpi][0]*bufrecv[-nei_loc[ci][tmpi]-1][sumtmp[-nei_loc[ci][tmpi]-1]];
               C[1]+=3.0/(lat_c*lat_c*dt)*w[tmpi]*elat[tmpi][1]*bufrecv[-nei_loc[ci][tmpi]-1][sumtmp[-nei_loc[ci][tmpi]-1]];
               C[2]+=3.0/(lat_c*lat_c*dt)*w[tmpi]*elat[tmpi][2]*bufrecv[-nei_loc[ci][tmpi]-1][sumtmp[-nei_loc[ci][tmpi]-1]];  
			        	                   sumtmp[-nei_loc[ci][tmpi]-1]++;
			        	                }
			        	                      
		                
		        }
		
			//-------------------------------------------------------------------------
			
			
			
		
      			
      			
		
		}



		//cout<<"**     "<<rank<<"	"<<ci<<"	"<<Count+1<<endl;
		//cout<<"@@@@@@@@"<<endl;

		uu=u[ci][0]*u[ci][0]+u[ci][1]*u[ci][1]+u[ci][2]*u[ci][2];

		if ((sqrt((rho_r[ci]-rho_b[ci])*(rho_r[ci]-rho_b[ci]))>=0.9) and (ind_S==1))
		{C[0]=0;C[1]=0;C[2]=0;}

		//C[0]=0;C[1]=0;C[2]=0;
			cc=sqrt(C[0]*C[0]+C[1]*C[1]+C[2]*C[2]);
			if (cc>0)
			        {nx=C[0]/cc;ny=C[1]/cc;nz=C[2]/cc;}
			else
			        {nx=0;ny=0;nz=0;}



//==========================
if (((bodyforce_apply==1) and (psi[ci]<rel_perm_psi)) or ((bodyforce_apply==-1) and (psi[ci]>-rel_perm_psi)))
{
GuoF[0]=0.0;GuoF[1]=0.0;GuoF[2]=0.0;GuoF[3]=0.0;GuoF[4]=0.0;GuoF[5]=0.0;GuoF[6]=0.0;GuoF[7]=0.0;
GuoF[8]=0.0;GuoF[9]=0.0;GuoF[10]=0.0;GuoF[11]=0.0;GuoF[12]=0.0;GuoF[13]=0.0;GuoF[14]=0.0;GuoF[15]=0.0;
GuoF[16]=0.0;GuoF[17]=0.0;GuoF[18]=0.0;
}
else
{
lm0=((+0.000*c_l-u[ci][0])*gxs+(+0.000*c_l-u[ci][1])*gys+(+0.000*c_l-u[ci][2])*gzs)/c_s2;
lm1=(+0.000*c_l*u[ci][0]+(+0.000*c_l)*u[ci][1]+(+0.000*c_l)*u[ci][2])*(+0.000*c_l*gxs+(+0.000*c_l)*gys+(+0.000*c_l)*gzs)/(c_s2*c_s2);
GuoF[0]=w[0]*(lm0+lm1);

lm0=((+1.000*c_l-u[ci][0])*gxs+(+0.000*c_l-u[ci][1])*gys+(+0.000*c_l-u[ci][2])*gzs)/c_s2;
lm1=(+1.000*c_l*u[ci][0]+(+0.000*c_l)*u[ci][1]+(+0.000*c_l)*u[ci][2])*(+1.000*c_l*gxs+(+0.000*c_l)*gys+(+0.000*c_l)*gzs)/(c_s2*c_s2);
GuoF[1]=w[1]*(lm0+lm1);

lm0=((-1.000*c_l-u[ci][0])*gxs+(+0.000*c_l-u[ci][1])*gys+(+0.000*c_l-u[ci][2])*gzs)/c_s2;
lm1=(-1.000*c_l*u[ci][0]+(+0.000*c_l)*u[ci][1]+(+0.000*c_l)*u[ci][2])*(-1.000*c_l*gxs+(+0.000*c_l)*gys+(+0.000*c_l)*gzs)/(c_s2*c_s2);
GuoF[2]=w[2]*(lm0+lm1);

lm0=((+0.000*c_l-u[ci][0])*gxs+(+1.000*c_l-u[ci][1])*gys+(+0.000*c_l-u[ci][2])*gzs)/c_s2;
lm1=(+0.000*c_l*u[ci][0]+(+1.000*c_l)*u[ci][1]+(+0.000*c_l)*u[ci][2])*(+0.000*c_l*gxs+(+1.000*c_l)*gys+(+0.000*c_l)*gzs)/(c_s2*c_s2);
GuoF[3]=w[3]*(lm0+lm1);

lm0=((+0.000*c_l-u[ci][0])*gxs+(-1.000*c_l-u[ci][1])*gys+(+0.000*c_l-u[ci][2])*gzs)/c_s2;
lm1=(+0.000*c_l*u[ci][0]+(-1.000*c_l)*u[ci][1]+(+0.000*c_l)*u[ci][2])*(+0.000*c_l*gxs+(-1.000*c_l)*gys+(+0.000*c_l)*gzs)/(c_s2*c_s2);
GuoF[4]=w[4]*(lm0+lm1);

lm0=((+0.000*c_l-u[ci][0])*gxs+(+0.000*c_l-u[ci][1])*gys+(+1.000*c_l-u[ci][2])*gzs)/c_s2;
lm1=(+0.000*c_l*u[ci][0]+(+0.000*c_l)*u[ci][1]+(+1.000*c_l)*u[ci][2])*(+0.000*c_l*gxs+(+0.000*c_l)*gys+(+1.000*c_l)*gzs)/(c_s2*c_s2);
GuoF[5]=w[5]*(lm0+lm1);

lm0=((+0.000*c_l-u[ci][0])*gxs+(+0.000*c_l-u[ci][1])*gys+(-1.000*c_l-u[ci][2])*gzs)/c_s2;
lm1=(+0.000*c_l*u[ci][0]+(+0.000*c_l)*u[ci][1]+(-1.000*c_l)*u[ci][2])*(+0.000*c_l*gxs+(+0.000*c_l)*gys+(-1.000*c_l)*gzs)/(c_s2*c_s2);
GuoF[6]=w[6]*(lm0+lm1);

lm0=((+1.000*c_l-u[ci][0])*gxs+(+1.000*c_l-u[ci][1])*gys+(+0.000*c_l-u[ci][2])*gzs)/c_s2;
lm1=(+1.000*c_l*u[ci][0]+(+1.000*c_l)*u[ci][1]+(+0.000*c_l)*u[ci][2])*(+1.000*c_l*gxs+(+1.000*c_l)*gys+(+0.000*c_l)*gzs)/(c_s2*c_s2);
GuoF[7]=w[7]*(lm0+lm1);

lm0=((-1.000*c_l-u[ci][0])*gxs+(-1.000*c_l-u[ci][1])*gys+(+0.000*c_l-u[ci][2])*gzs)/c_s2;
lm1=(-1.000*c_l*u[ci][0]+(-1.000*c_l)*u[ci][1]+(+0.000*c_l)*u[ci][2])*(-1.000*c_l*gxs+(-1.000*c_l)*gys+(+0.000*c_l)*gzs)/(c_s2*c_s2);
GuoF[8]=w[8]*(lm0+lm1);

lm0=((+1.000*c_l-u[ci][0])*gxs+(-1.000*c_l-u[ci][1])*gys+(+0.000*c_l-u[ci][2])*gzs)/c_s2;
lm1=(+1.000*c_l*u[ci][0]+(-1.000*c_l)*u[ci][1]+(+0.000*c_l)*u[ci][2])*(+1.000*c_l*gxs+(-1.000*c_l)*gys+(+0.000*c_l)*gzs)/(c_s2*c_s2);
GuoF[9]=w[9]*(lm0+lm1);

lm0=((-1.000*c_l-u[ci][0])*gxs+(+1.000*c_l-u[ci][1])*gys+(+0.000*c_l-u[ci][2])*gzs)/c_s2;
lm1=(-1.000*c_l*u[ci][0]+(+1.000*c_l)*u[ci][1]+(+0.000*c_l)*u[ci][2])*(-1.000*c_l*gxs+(+1.000*c_l)*gys+(+0.000*c_l)*gzs)/(c_s2*c_s2);
GuoF[10]=w[10]*(lm0+lm1);

lm0=((+1.000*c_l-u[ci][0])*gxs+(+0.000*c_l-u[ci][1])*gys+(+1.000*c_l-u[ci][2])*gzs)/c_s2;
lm1=(+1.000*c_l*u[ci][0]+(+0.000*c_l)*u[ci][1]+(+1.000*c_l)*u[ci][2])*(+1.000*c_l*gxs+(+0.000*c_l)*gys+(+1.000*c_l)*gzs)/(c_s2*c_s2);
GuoF[11]=w[11]*(lm0+lm1);

lm0=((-1.000*c_l-u[ci][0])*gxs+(+0.000*c_l-u[ci][1])*gys+(-1.000*c_l-u[ci][2])*gzs)/c_s2;
lm1=(-1.000*c_l*u[ci][0]+(+0.000*c_l)*u[ci][1]+(-1.000*c_l)*u[ci][2])*(-1.000*c_l*gxs+(+0.000*c_l)*gys+(-1.000*c_l)*gzs)/(c_s2*c_s2);
GuoF[12]=w[12]*(lm0+lm1);

lm0=((+1.000*c_l-u[ci][0])*gxs+(+0.000*c_l-u[ci][1])*gys+(-1.000*c_l-u[ci][2])*gzs)/c_s2;
lm1=(+1.000*c_l*u[ci][0]+(+0.000*c_l)*u[ci][1]+(-1.000*c_l)*u[ci][2])*(+1.000*c_l*gxs+(+0.000*c_l)*gys+(-1.000*c_l)*gzs)/(c_s2*c_s2);
GuoF[13]=w[13]*(lm0+lm1);

lm0=((-1.000*c_l-u[ci][0])*gxs+(+0.000*c_l-u[ci][1])*gys+(+1.000*c_l-u[ci][2])*gzs)/c_s2;
lm1=(-1.000*c_l*u[ci][0]+(+0.000*c_l)*u[ci][1]+(+1.000*c_l)*u[ci][2])*(-1.000*c_l*gxs+(+0.000*c_l)*gys+(+1.000*c_l)*gzs)/(c_s2*c_s2);
GuoF[14]=w[14]*(lm0+lm1);

lm0=((+0.000*c_l-u[ci][0])*gxs+(+1.000*c_l-u[ci][1])*gys+(+1.000*c_l-u[ci][2])*gzs)/c_s2;
lm1=(+0.000*c_l*u[ci][0]+(+1.000*c_l)*u[ci][1]+(+1.000*c_l)*u[ci][2])*(+0.000*c_l*gxs+(+1.000*c_l)*gys+(+1.000*c_l)*gzs)/(c_s2*c_s2);
GuoF[15]=w[15]*(lm0+lm1);

lm0=((+0.000*c_l-u[ci][0])*gxs+(-1.000*c_l-u[ci][1])*gys+(-1.000*c_l-u[ci][2])*gzs)/c_s2;
lm1=(+0.000*c_l*u[ci][0]+(-1.000*c_l)*u[ci][1]+(-1.000*c_l)*u[ci][2])*(+0.000*c_l*gxs+(-1.000*c_l)*gys+(-1.000*c_l)*gzs)/(c_s2*c_s2);
GuoF[16]=w[16]*(lm0+lm1);

lm0=((+0.000*c_l-u[ci][0])*gxs+(+1.000*c_l-u[ci][1])*gys+(-1.000*c_l-u[ci][2])*gzs)/c_s2;
lm1=(+0.000*c_l*u[ci][0]+(+1.000*c_l)*u[ci][1]+(-1.000*c_l)*u[ci][2])*(+0.000*c_l*gxs+(+1.000*c_l)*gys+(-1.000*c_l)*gzs)/(c_s2*c_s2);
GuoF[17]=w[17]*(lm0+lm1);

lm0=((+0.000*c_l-u[ci][0])*gxs+(-1.000*c_l-u[ci][1])*gys+(+1.000*c_l-u[ci][2])*gzs)/c_s2;
lm1=(+0.000*c_l*u[ci][0]+(-1.000*c_l)*u[ci][1]+(+1.000*c_l)*u[ci][2])*(+0.000*c_l*gxs+(-1.000*c_l)*gys+(+1.000*c_l)*gzs)/(c_s2*c_s2);
GuoF[18]=w[18]*(lm0+lm1);
}
//====================

			//if (n>2100)
			//cout<<c_s2<<"	"<<rank<<endl;
			//=====================equilibrium of moment=================================
			ux=u[ci][0];
			uy=u[ci][1];
			uz=u[ci][2];
			
			for(int k=0;k<19;k++)
				meq[k]=0;

			
	//========================================================================================		
			meq[0]=rho[ci];meq[3]=rho_0*ux;meq[5]=rho_0*uy;meq[7]=rho_0*uz;
			meq[1]=rho_0*(ux*ux+uy*uy+uz*uz)+CapA*cc;
			meq[9]=rho_0*(2*ux*ux-uy*uy-uz*uz)+0.5*CapA*cc*(2*nx*nx-ny*ny-nz*nz);
			meq[11]=rho_0*(uy*uy-uz*uz)+0.5*CapA*cc*(ny*ny-nz*nz);
			meq[13]=rho_0*ux*uy+0.5*CapA*cc*(nx*ny);
			meq[14]=rho_0*uy*uz+0.5*CapA*cc*(ny*nz);
			meq[15]=rho_0*ux*uz+0.5*CapA*cc*(nx*nz);
	//========================================================================================

	
	//=========================================================================================
			if (mix_psi_thickness>0)
			if ((psi_xn>0) and (i<=zero_sigma_thickness))
			{
			meq[0]=rho[ci];meq[3]=rho_0*ux;meq[5]=rho_0*uy;meq[7]=rho_0*uz;
			meq[1]=rho_0*(ux*ux+uy*uy+uz*uz);
			meq[9]=rho_0*(2*ux*ux-uy*uy-uz*uz);
			meq[11]=rho_0*(uy*uy-uz*uz);
			meq[13]=rho_0*ux*uy;
			meq[14]=rho_0*uy*uz;
			meq[15]=rho_0*ux*uz;
			}

	//=======================================================================================		
			
			//s_v=niu_g+(psi[ci]+1.0)/2.0*(niu_l-niu_g);
			//s_v=1.0/(s_v/(c_s2*dt)+0.5);
			//s_other=8*(2-s_v)/(8-s_v);
			

		//================VISCOSITY INTERPOLATION============
			if (psi[ci]>0)
				if (psi[ci]>delta)
				s_v=wl;
				else
				s_v=lg0+l1*psi[ci]+l2*psi[ci]*psi[ci];
			else
				if (psi[ci]<-delta)
				s_v=wg;
				else
				s_v=lg0+g1*psi[ci]+g2*psi[ci]*psi[ci];					

			s_other=8*(2-s_v)/(8-s_v);
		//==================================================

	//if (n>2030) 
	//cout<<s_v<<"		"<<s_other<<"	"<<n<<endl;

	//if (s_v!=s_v)
	//	cout<<delta<<"	"<<psi[ci]<<"	"<<n<<endl;

	//cout<<"@@@@@@@@@   "<<s_v<<"  "<<C[0]<<"   "<<C[1]<<"  "<<C[2]<<endl;
	//cout<<"@@@@@@@@@   "<<s_v<<"  "<<ux<<"   "<<uy<<"  "<<uz<<"  "<<rho_r[ci]<<" "<<rho_b[ci]<<endl;
	
	
	S[1]=s_v;S[2]=s_v;S[4]=s_other;S[6]=s_other;S[8]=s_other;S[9]=s_v;
	S[10]=s_v;S[11]=s_v;S[12]=s_v;S[13]=s_v;S[14]=s_v;S[15]=s_v;S[16]=s_other;
	S[17]=s_other;S[18]=s_other;



			//============================================================================

			/*
			// ==================   m=Mf matrix calculation  =============================
			// ==================   F_hat=(I-.5*S)MGuoF =====================================
				for (int mi=0; mi<19; mi++)
					{m_l[mi]=0;F_hat[mi]=0;
					for (int mj=0; mj<19; mj++)
						{
						m_l[mi]+=M[mi][mj]*f[ci][mj];
						F_hat[mi]+=M[mi][mj]*GuoF[mj];
						}
					F_hat[mi]*=(1-0.5*S[mi]);
					m_l[mi]=m_l[mi]-S[mi]*(m_l[mi]-meq[mi])+dt*F_hat[mi];
					}
			//============================================================================
			*/


//==========================
m_l[0]=+1.000*1.0*f[ci][0]+1.00000000000000000*1.0*f[ci][1]+1.00000000000000000*1.0*f[ci][2]+1.00000000000000000*1.0*f[ci][3]+1.00000000000000000*1.0*f[ci][4]+1.00000000000000000*1.0*f[ci][5]+1.00000000000000000*1.0*f[ci][6]+1.00000000000000000*1.0*f[ci][7]+1.00000000000000000*1.0*f[ci][8]+1.00000000000000000*1.0*f[ci][9]+1.00000000000000000*1.0*f[ci][10]+1.00000000000000000*1.0*f[ci][11]+1.00000000000000000*1.0*f[ci][12]+1.00000000000000000*1.0*f[ci][13]+1.00000000000000000*1.0*f[ci][14]+1.00000000000000000*1.0*f[ci][15]+1.00000000000000000*1.0*f[ci][16]+1.00000000000000000*1.0*f[ci][17]+1.00000000000000000*1.0*f[ci][18];

F_hat[0]=+1.000*1.0*GuoF[0]+1.00000000000000000*1.0*GuoF[1]+1.00000000000000000*1.0*GuoF[2]+1.00000000000000000*1.0*GuoF[3]+1.00000000000000000*1.0*GuoF[4]+1.00000000000000000*1.0*GuoF[5]+1.00000000000000000*1.0*GuoF[6]+1.00000000000000000*1.0*GuoF[7]+1.00000000000000000*1.0*GuoF[8]+1.00000000000000000*1.0*GuoF[9]+1.00000000000000000*1.0*GuoF[10]+1.00000000000000000*1.0*GuoF[11]+1.00000000000000000*1.0*GuoF[12]+1.00000000000000000*1.0*GuoF[13]+1.00000000000000000*1.0*GuoF[14]+1.00000000000000000*1.0*GuoF[15]+1.00000000000000000*1.0*GuoF[16]+1.00000000000000000*1.0*GuoF[17]+1.00000000000000000*1.0*GuoF[18];

F_hat[0]*=(1-0.5*S[0]);
m_l[0]=m_l[0]-S[0]*(m_l[0]-meq[0])+dt*F_hat[0];
//=======================================

m_l[1]=-1.00000000000000000*c_l*c_l*f[ci][0]+0.00000000000000000*c_l*c_l*f[ci][1]+0.00000000000000000*c_l*c_l*f[ci][2]+0.00000000000000000*c_l*c_l*f[ci][3]+0.00000000000000000*c_l*c_l*f[ci][4]+0.00000000000000000*c_l*c_l*f[ci][5]+0.00000000000000000*c_l*c_l*f[ci][6]+1.00000000000000000*c_l*c_l*f[ci][7]+1.00000000000000000*c_l*c_l*f[ci][8]+1.00000000000000000*c_l*c_l*f[ci][9]+1.00000000000000000*c_l*c_l*f[ci][10]+1.00000000000000000*c_l*c_l*f[ci][11]+1.00000000000000000*c_l*c_l*f[ci][12]+1.00000000000000000*c_l*c_l*f[ci][13]+1.00000000000000000*c_l*c_l*f[ci][14]+1.00000000000000000*c_l*c_l*f[ci][15]+1.00000000000000000*c_l*c_l*f[ci][16]+1.00000000000000000*c_l*c_l*f[ci][17]+1.00000000000000000*c_l*c_l*f[ci][18];

F_hat[1]=-1.00000000000000000*c_l*c_l*GuoF[0]+0.00000000000000000*c_l*c_l*GuoF[1]+0.00000000000000000*c_l*c_l*GuoF[2]+0.00000000000000000*c_l*c_l*GuoF[3]+0.00000000000000000*c_l*c_l*GuoF[4]+0.00000000000000000*c_l*c_l*GuoF[5]+0.00000000000000000*c_l*c_l*GuoF[6]+1.00000000000000000*c_l*c_l*GuoF[7]+1.00000000000000000*c_l*c_l*GuoF[8]+1.00000000000000000*c_l*c_l*GuoF[9]+1.00000000000000000*c_l*c_l*GuoF[10]+1.00000000000000000*c_l*c_l*GuoF[11]+1.00000000000000000*c_l*c_l*GuoF[12]+1.00000000000000000*c_l*c_l*GuoF[13]+1.00000000000000000*c_l*c_l*GuoF[14]+1.00000000000000000*c_l*c_l*GuoF[15]+1.00000000000000000*c_l*c_l*GuoF[16]+1.00000000000000000*c_l*c_l*GuoF[17]+1.00000000000000000*c_l*c_l*GuoF[18];

F_hat[1]*=(1-0.5*S[1]);
m_l[1]=m_l[1]-S[1]*(m_l[1]-meq[1])+dt*F_hat[1];
//=======================================

m_l[2]=+1.00000000000000000*c_l*c_l*c_l*c_l*f[ci][0]-2.00000000000000000*c_l*c_l*c_l*c_l*f[ci][1]-2.00000000000000000*c_l*c_l*c_l*c_l*f[ci][2]-2.00000000000000000*c_l*c_l*c_l*c_l*f[ci][3]-2.00000000000000000*c_l*c_l*c_l*c_l*f[ci][4]-2.00000000000000000*c_l*c_l*c_l*c_l*f[ci][5]-2.00000000000000000*c_l*c_l*c_l*c_l*f[ci][6]+1.00000000000000000*c_l*c_l*c_l*c_l*f[ci][7]+1.00000000000000000*c_l*c_l*c_l*c_l*f[ci][8]+1.00000000000000000*c_l*c_l*c_l*c_l*f[ci][9]+1.00000000000000000*c_l*c_l*c_l*c_l*f[ci][10]+1.00000000000000000*c_l*c_l*c_l*c_l*f[ci][11]+1.00000000000000000*c_l*c_l*c_l*c_l*f[ci][12]+1.00000000000000000*c_l*c_l*c_l*c_l*f[ci][13]+1.00000000000000000*c_l*c_l*c_l*c_l*f[ci][14]+1.00000000000000000*c_l*c_l*c_l*c_l*f[ci][15]+1.00000000000000000*c_l*c_l*c_l*c_l*f[ci][16]+1.00000000000000000*c_l*c_l*c_l*c_l*f[ci][17]+1.00000000000000000*c_l*c_l*c_l*c_l*f[ci][18];

F_hat[2]=+1.00000000000000000*c_l*c_l*c_l*c_l*GuoF[0]-2.00000000000000000*c_l*c_l*c_l*c_l*GuoF[1]-2.00000000000000000*c_l*c_l*c_l*c_l*GuoF[2]-2.00000000000000000*c_l*c_l*c_l*c_l*GuoF[3]-2.00000000000000000*c_l*c_l*c_l*c_l*GuoF[4]-2.00000000000000000*c_l*c_l*c_l*c_l*GuoF[5]-2.00000000000000000*c_l*c_l*c_l*c_l*GuoF[6]+1.00000000000000000*c_l*c_l*c_l*c_l*GuoF[7]+1.00000000000000000*c_l*c_l*c_l*c_l*GuoF[8]+1.00000000000000000*c_l*c_l*c_l*c_l*GuoF[9]+1.00000000000000000*c_l*c_l*c_l*c_l*GuoF[10]+1.00000000000000000*c_l*c_l*c_l*c_l*GuoF[11]+1.00000000000000000*c_l*c_l*c_l*c_l*GuoF[12]+1.00000000000000000*c_l*c_l*c_l*c_l*GuoF[13]+1.00000000000000000*c_l*c_l*c_l*c_l*GuoF[14]+1.00000000000000000*c_l*c_l*c_l*c_l*GuoF[15]+1.00000000000000000*c_l*c_l*c_l*c_l*GuoF[16]+1.00000000000000000*c_l*c_l*c_l*c_l*GuoF[17]+1.00000000000000000*c_l*c_l*c_l*c_l*GuoF[18];

F_hat[2]*=(1-0.5*S[2]);
m_l[2]=m_l[2]-S[2]*(m_l[2]-meq[2])+dt*F_hat[2];
//=======================================

m_l[3]=+0.00000000000000000*c_l*f[ci][0]+1.00000000000000000*c_l*f[ci][1]-1.00000000000000000*c_l*f[ci][2]+0.00000000000000000*c_l*f[ci][3]+0.00000000000000000*c_l*f[ci][4]+0.00000000000000000*c_l*f[ci][5]+0.00000000000000000*c_l*f[ci][6]+1.00000000000000000*c_l*f[ci][7]-1.00000000000000000*c_l*f[ci][8]+1.00000000000000000*c_l*f[ci][9]-1.00000000000000000*c_l*f[ci][10]+1.00000000000000000*c_l*f[ci][11]-1.00000000000000000*c_l*f[ci][12]+1.00000000000000000*c_l*f[ci][13]-1.00000000000000000*c_l*f[ci][14]+0.00000000000000000*c_l*f[ci][15]+0.00000000000000000*c_l*f[ci][16]+0.00000000000000000*c_l*f[ci][17]+0.00000000000000000*c_l*f[ci][18];

F_hat[3]=+0.00000000000000000*c_l*GuoF[0]+1.00000000000000000*c_l*GuoF[1]-1.00000000000000000*c_l*GuoF[2]+0.00000000000000000*c_l*GuoF[3]+0.00000000000000000*c_l*GuoF[4]+0.00000000000000000*c_l*GuoF[5]+0.00000000000000000*c_l*GuoF[6]+1.00000000000000000*c_l*GuoF[7]-1.00000000000000000*c_l*GuoF[8]+1.00000000000000000*c_l*GuoF[9]-1.00000000000000000*c_l*GuoF[10]+1.00000000000000000*c_l*GuoF[11]-1.00000000000000000*c_l*GuoF[12]+1.00000000000000000*c_l*GuoF[13]-1.00000000000000000*c_l*GuoF[14]+0.00000000000000000*c_l*GuoF[15]+0.00000000000000000*c_l*GuoF[16]+0.00000000000000000*c_l*GuoF[17]+0.00000000000000000*c_l*GuoF[18];

F_hat[3]*=(1-0.5*S[3]);
m_l[3]=m_l[3]-S[3]*(m_l[3]-meq[3])+dt*F_hat[3];
//=======================================

m_l[4]=+0.00000000000000000*c_l*c_l*c_l*f[ci][0]-2.00000000000000000*c_l*c_l*c_l*f[ci][1]+2.00000000000000000*c_l*c_l*c_l*f[ci][2]+0.00000000000000000*c_l*c_l*c_l*f[ci][3]+0.00000000000000000*c_l*c_l*c_l*f[ci][4]+0.00000000000000000*c_l*c_l*c_l*f[ci][5]+0.00000000000000000*c_l*c_l*c_l*f[ci][6]+1.00000000000000000*c_l*c_l*c_l*f[ci][7]-1.00000000000000000*c_l*c_l*c_l*f[ci][8]+1.00000000000000000*c_l*c_l*c_l*f[ci][9]-1.00000000000000000*c_l*c_l*c_l*f[ci][10]+1.00000000000000000*c_l*c_l*c_l*f[ci][11]-1.00000000000000000*c_l*c_l*c_l*f[ci][12]+1.00000000000000000*c_l*c_l*c_l*f[ci][13]-1.00000000000000000*c_l*c_l*c_l*f[ci][14]+0.00000000000000000*c_l*c_l*c_l*f[ci][15]+0.00000000000000000*c_l*c_l*c_l*f[ci][16]+0.00000000000000000*c_l*c_l*c_l*f[ci][17]+0.00000000000000000*c_l*c_l*c_l*f[ci][18];

F_hat[4]=+0.00000000000000000*c_l*c_l*c_l*GuoF[0]-2.00000000000000000*c_l*c_l*c_l*GuoF[1]+2.00000000000000000*c_l*c_l*c_l*GuoF[2]+0.00000000000000000*c_l*c_l*c_l*GuoF[3]+0.00000000000000000*c_l*c_l*c_l*GuoF[4]+0.00000000000000000*c_l*c_l*c_l*GuoF[5]+0.00000000000000000*c_l*c_l*c_l*GuoF[6]+1.00000000000000000*c_l*c_l*c_l*GuoF[7]-1.00000000000000000*c_l*c_l*c_l*GuoF[8]+1.00000000000000000*c_l*c_l*c_l*GuoF[9]-1.00000000000000000*c_l*c_l*c_l*GuoF[10]+1.00000000000000000*c_l*c_l*c_l*GuoF[11]-1.00000000000000000*c_l*c_l*c_l*GuoF[12]+1.00000000000000000*c_l*c_l*c_l*GuoF[13]-1.00000000000000000*c_l*c_l*c_l*GuoF[14]+0.00000000000000000*c_l*c_l*c_l*GuoF[15]+0.00000000000000000*c_l*c_l*c_l*GuoF[16]+0.00000000000000000*c_l*c_l*c_l*GuoF[17]+0.00000000000000000*c_l*c_l*c_l*GuoF[18];

F_hat[4]*=(1-0.5*S[4]);
m_l[4]=m_l[4]-S[4]*(m_l[4]-meq[4])+dt*F_hat[4];
//=======================================

m_l[5]=+0.00000000000000000*c_l*f[ci][0]+0.00000000000000000*c_l*f[ci][1]+0.00000000000000000*c_l*f[ci][2]+1.00000000000000000*c_l*f[ci][3]-1.00000000000000000*c_l*f[ci][4]+0.00000000000000000*c_l*f[ci][5]+0.00000000000000000*c_l*f[ci][6]+1.00000000000000000*c_l*f[ci][7]-1.00000000000000000*c_l*f[ci][8]-1.00000000000000000*c_l*f[ci][9]+1.00000000000000000*c_l*f[ci][10]+0.00000000000000000*c_l*f[ci][11]+0.00000000000000000*c_l*f[ci][12]+0.00000000000000000*c_l*f[ci][13]+0.00000000000000000*c_l*f[ci][14]+1.00000000000000000*c_l*f[ci][15]-1.00000000000000000*c_l*f[ci][16]+1.00000000000000000*c_l*f[ci][17]-1.00000000000000000*c_l*f[ci][18];

F_hat[5]=+0.00000000000000000*c_l*GuoF[0]+0.00000000000000000*c_l*GuoF[1]+0.00000000000000000*c_l*GuoF[2]+1.00000000000000000*c_l*GuoF[3]-1.00000000000000000*c_l*GuoF[4]+0.00000000000000000*c_l*GuoF[5]+0.00000000000000000*c_l*GuoF[6]+1.00000000000000000*c_l*GuoF[7]-1.00000000000000000*c_l*GuoF[8]-1.00000000000000000*c_l*GuoF[9]+1.00000000000000000*c_l*GuoF[10]+0.00000000000000000*c_l*GuoF[11]+0.00000000000000000*c_l*GuoF[12]+0.00000000000000000*c_l*GuoF[13]+0.00000000000000000*c_l*GuoF[14]+1.00000000000000000*c_l*GuoF[15]-1.00000000000000000*c_l*GuoF[16]+1.00000000000000000*c_l*GuoF[17]-1.00000000000000000*c_l*GuoF[18];

F_hat[5]*=(1-0.5*S[5]);
m_l[5]=m_l[5]-S[5]*(m_l[5]-meq[5])+dt*F_hat[5];
//=======================================

m_l[6]=+0.00000000000000000*c_l*c_l*c_l*f[ci][0]+0.00000000000000000*c_l*c_l*c_l*f[ci][1]+0.00000000000000000*c_l*c_l*c_l*f[ci][2]-2.00000000000000000*c_l*c_l*c_l*f[ci][3]+2.00000000000000000*c_l*c_l*c_l*f[ci][4]+0.00000000000000000*c_l*c_l*c_l*f[ci][5]+0.00000000000000000*c_l*c_l*c_l*f[ci][6]+1.00000000000000000*c_l*c_l*c_l*f[ci][7]-1.00000000000000000*c_l*c_l*c_l*f[ci][8]-1.00000000000000000*c_l*c_l*c_l*f[ci][9]+1.00000000000000000*c_l*c_l*c_l*f[ci][10]+0.00000000000000000*c_l*c_l*c_l*f[ci][11]+0.00000000000000000*c_l*c_l*c_l*f[ci][12]+0.00000000000000000*c_l*c_l*c_l*f[ci][13]+0.00000000000000000*c_l*c_l*c_l*f[ci][14]+1.00000000000000000*c_l*c_l*c_l*f[ci][15]-1.00000000000000000*c_l*c_l*c_l*f[ci][16]+1.00000000000000000*c_l*c_l*c_l*f[ci][17]-1.00000000000000000*c_l*c_l*c_l*f[ci][18];

F_hat[6]=+0.00000000000000000*c_l*c_l*c_l*GuoF[0]+0.00000000000000000*c_l*c_l*c_l*GuoF[1]+0.00000000000000000*c_l*c_l*c_l*GuoF[2]-2.00000000000000000*c_l*c_l*c_l*GuoF[3]+2.00000000000000000*c_l*c_l*c_l*GuoF[4]+0.00000000000000000*c_l*c_l*c_l*GuoF[5]+0.00000000000000000*c_l*c_l*c_l*GuoF[6]+1.00000000000000000*c_l*c_l*c_l*GuoF[7]-1.00000000000000000*c_l*c_l*c_l*GuoF[8]-1.00000000000000000*c_l*c_l*c_l*GuoF[9]+1.00000000000000000*c_l*c_l*c_l*GuoF[10]+0.00000000000000000*c_l*c_l*c_l*GuoF[11]+0.00000000000000000*c_l*c_l*c_l*GuoF[12]+0.00000000000000000*c_l*c_l*c_l*GuoF[13]+0.00000000000000000*c_l*c_l*c_l*GuoF[14]+1.00000000000000000*c_l*c_l*c_l*GuoF[15]-1.00000000000000000*c_l*c_l*c_l*GuoF[16]+1.00000000000000000*c_l*c_l*c_l*GuoF[17]-1.00000000000000000*c_l*c_l*c_l*GuoF[18];

F_hat[6]*=(1-0.5*S[6]);
m_l[6]=m_l[6]-S[6]*(m_l[6]-meq[6])+dt*F_hat[6];
//=======================================

m_l[7]=+0.00000000000000000*c_l*f[ci][0]+0.00000000000000000*c_l*f[ci][1]+0.00000000000000000*c_l*f[ci][2]+0.00000000000000000*c_l*f[ci][3]+0.00000000000000000*c_l*f[ci][4]+1.00000000000000000*c_l*f[ci][5]-1.00000000000000000*c_l*f[ci][6]+0.00000000000000000*c_l*f[ci][7]+0.00000000000000000*c_l*f[ci][8]+0.00000000000000000*c_l*f[ci][9]+0.00000000000000000*c_l*f[ci][10]+1.00000000000000000*c_l*f[ci][11]-1.00000000000000000*c_l*f[ci][12]-1.00000000000000000*c_l*f[ci][13]+1.00000000000000000*c_l*f[ci][14]+1.00000000000000000*c_l*f[ci][15]-1.00000000000000000*c_l*f[ci][16]-1.00000000000000000*c_l*f[ci][17]+1.00000000000000000*c_l*f[ci][18];

F_hat[7]=+0.00000000000000000*c_l*GuoF[0]+0.00000000000000000*c_l*GuoF[1]+0.00000000000000000*c_l*GuoF[2]+0.00000000000000000*c_l*GuoF[3]+0.00000000000000000*c_l*GuoF[4]+1.00000000000000000*c_l*GuoF[5]-1.00000000000000000*c_l*GuoF[6]+0.00000000000000000*c_l*GuoF[7]+0.00000000000000000*c_l*GuoF[8]+0.00000000000000000*c_l*GuoF[9]+0.00000000000000000*c_l*GuoF[10]+1.00000000000000000*c_l*GuoF[11]-1.00000000000000000*c_l*GuoF[12]-1.00000000000000000*c_l*GuoF[13]+1.00000000000000000*c_l*GuoF[14]+1.00000000000000000*c_l*GuoF[15]-1.00000000000000000*c_l*GuoF[16]-1.00000000000000000*c_l*GuoF[17]+1.00000000000000000*c_l*GuoF[18];

F_hat[7]*=(1-0.5*S[7]);
m_l[7]=m_l[7]-S[7]*(m_l[7]-meq[7])+dt*F_hat[7];
//=======================================

m_l[8]=+0.00000000000000000*c_l*c_l*c_l*f[ci][0]+0.00000000000000000*c_l*c_l*c_l*f[ci][1]+0.00000000000000000*c_l*c_l*c_l*f[ci][2]+0.00000000000000000*c_l*c_l*c_l*f[ci][3]+0.00000000000000000*c_l*c_l*c_l*f[ci][4]-2.00000000000000000*c_l*c_l*c_l*f[ci][5]+2.00000000000000000*c_l*c_l*c_l*f[ci][6]+0.00000000000000000*c_l*c_l*c_l*f[ci][7]+0.00000000000000000*c_l*c_l*c_l*f[ci][8]+0.00000000000000000*c_l*c_l*c_l*f[ci][9]+0.00000000000000000*c_l*c_l*c_l*f[ci][10]+1.00000000000000000*c_l*c_l*c_l*f[ci][11]-1.00000000000000000*c_l*c_l*c_l*f[ci][12]-1.00000000000000000*c_l*c_l*c_l*f[ci][13]+1.00000000000000000*c_l*c_l*c_l*f[ci][14]+1.00000000000000000*c_l*c_l*c_l*f[ci][15]-1.00000000000000000*c_l*c_l*c_l*f[ci][16]-1.00000000000000000*c_l*c_l*c_l*f[ci][17]+1.00000000000000000*c_l*c_l*c_l*f[ci][18];

F_hat[8]=+0.00000000000000000*c_l*c_l*c_l*GuoF[0]+0.00000000000000000*c_l*c_l*c_l*GuoF[1]+0.00000000000000000*c_l*c_l*c_l*GuoF[2]+0.00000000000000000*c_l*c_l*c_l*GuoF[3]+0.00000000000000000*c_l*c_l*c_l*GuoF[4]-2.00000000000000000*c_l*c_l*c_l*GuoF[5]+2.00000000000000000*c_l*c_l*c_l*GuoF[6]+0.00000000000000000*c_l*c_l*c_l*GuoF[7]+0.00000000000000000*c_l*c_l*c_l*GuoF[8]+0.00000000000000000*c_l*c_l*c_l*GuoF[9]+0.00000000000000000*c_l*c_l*c_l*GuoF[10]+1.00000000000000000*c_l*c_l*c_l*GuoF[11]-1.00000000000000000*c_l*c_l*c_l*GuoF[12]-1.00000000000000000*c_l*c_l*c_l*GuoF[13]+1.00000000000000000*c_l*c_l*c_l*GuoF[14]+1.00000000000000000*c_l*c_l*c_l*GuoF[15]-1.00000000000000000*c_l*c_l*c_l*GuoF[16]-1.00000000000000000*c_l*c_l*c_l*GuoF[17]+1.00000000000000000*c_l*c_l*c_l*GuoF[18];

F_hat[8]*=(1-0.5*S[8]);
m_l[8]=m_l[8]-S[8]*(m_l[8]-meq[8])+dt*F_hat[8];
//=======================================

m_l[9]=+0.00000000000000000*c_l*c_l*f[ci][0]+2.00000000000000000*c_l*c_l*f[ci][1]+2.00000000000000000*c_l*c_l*f[ci][2]-1.00000000000000000*c_l*c_l*f[ci][3]-1.00000000000000000*c_l*c_l*f[ci][4]-1.00000000000000000*c_l*c_l*f[ci][5]-1.00000000000000000*c_l*c_l*f[ci][6]+1.00000000000000000*c_l*c_l*f[ci][7]+1.00000000000000000*c_l*c_l*f[ci][8]+1.00000000000000000*c_l*c_l*f[ci][9]+1.00000000000000000*c_l*c_l*f[ci][10]+1.00000000000000000*c_l*c_l*f[ci][11]+1.00000000000000000*c_l*c_l*f[ci][12]+1.00000000000000000*c_l*c_l*f[ci][13]+1.00000000000000000*c_l*c_l*f[ci][14]-2.00000000000000000*c_l*c_l*f[ci][15]-2.00000000000000000*c_l*c_l*f[ci][16]-2.00000000000000000*c_l*c_l*f[ci][17]-2.00000000000000000*c_l*c_l*f[ci][18];

F_hat[9]=+0.00000000000000000*c_l*c_l*GuoF[0]+2.00000000000000000*c_l*c_l*GuoF[1]+2.00000000000000000*c_l*c_l*GuoF[2]-1.00000000000000000*c_l*c_l*GuoF[3]-1.00000000000000000*c_l*c_l*GuoF[4]-1.00000000000000000*c_l*c_l*GuoF[5]-1.00000000000000000*c_l*c_l*GuoF[6]+1.00000000000000000*c_l*c_l*GuoF[7]+1.00000000000000000*c_l*c_l*GuoF[8]+1.00000000000000000*c_l*c_l*GuoF[9]+1.00000000000000000*c_l*c_l*GuoF[10]+1.00000000000000000*c_l*c_l*GuoF[11]+1.00000000000000000*c_l*c_l*GuoF[12]+1.00000000000000000*c_l*c_l*GuoF[13]+1.00000000000000000*c_l*c_l*GuoF[14]-2.00000000000000000*c_l*c_l*GuoF[15]-2.00000000000000000*c_l*c_l*GuoF[16]-2.00000000000000000*c_l*c_l*GuoF[17]-2.00000000000000000*c_l*c_l*GuoF[18];

F_hat[9]*=(1-0.5*S[9]);
m_l[9]=m_l[9]-S[9]*(m_l[9]-meq[9])+dt*F_hat[9];
//=======================================

m_l[10]=+0.00000000000000000*c_l*c_l*c_l*c_l*f[ci][0]-2.00000000000000000*c_l*c_l*c_l*c_l*f[ci][1]-2.00000000000000000*c_l*c_l*c_l*c_l*f[ci][2]+1.00000000000000000*c_l*c_l*c_l*c_l*f[ci][3]+1.00000000000000000*c_l*c_l*c_l*c_l*f[ci][4]+1.00000000000000000*c_l*c_l*c_l*c_l*f[ci][5]+1.00000000000000000*c_l*c_l*c_l*c_l*f[ci][6]+1.00000000000000000*c_l*c_l*c_l*c_l*f[ci][7]+1.00000000000000000*c_l*c_l*c_l*c_l*f[ci][8]+1.00000000000000000*c_l*c_l*c_l*c_l*f[ci][9]+1.00000000000000000*c_l*c_l*c_l*c_l*f[ci][10]+1.00000000000000000*c_l*c_l*c_l*c_l*f[ci][11]+1.00000000000000000*c_l*c_l*c_l*c_l*f[ci][12]+1.00000000000000000*c_l*c_l*c_l*c_l*f[ci][13]+1.00000000000000000*c_l*c_l*c_l*c_l*f[ci][14]-2.00000000000000000*c_l*c_l*c_l*c_l*f[ci][15]-2.00000000000000000*c_l*c_l*c_l*c_l*f[ci][16]-2.00000000000000000*c_l*c_l*c_l*c_l*f[ci][17]-2.00000000000000000*c_l*c_l*c_l*c_l*f[ci][18];

F_hat[10]=+0.00000000000000000*c_l*c_l*c_l*c_l*GuoF[0]-2.00000000000000000*c_l*c_l*c_l*c_l*GuoF[1]-2.00000000000000000*c_l*c_l*c_l*c_l*GuoF[2]+1.00000000000000000*c_l*c_l*c_l*c_l*GuoF[3]+1.00000000000000000*c_l*c_l*c_l*c_l*GuoF[4]+1.00000000000000000*c_l*c_l*c_l*c_l*GuoF[5]+1.00000000000000000*c_l*c_l*c_l*c_l*GuoF[6]+1.00000000000000000*c_l*c_l*c_l*c_l*GuoF[7]+1.00000000000000000*c_l*c_l*c_l*c_l*GuoF[8]+1.00000000000000000*c_l*c_l*c_l*c_l*GuoF[9]+1.00000000000000000*c_l*c_l*c_l*c_l*GuoF[10]+1.00000000000000000*c_l*c_l*c_l*c_l*GuoF[11]+1.00000000000000000*c_l*c_l*c_l*c_l*GuoF[12]+1.00000000000000000*c_l*c_l*c_l*c_l*GuoF[13]+1.00000000000000000*c_l*c_l*c_l*c_l*GuoF[14]-2.00000000000000000*c_l*c_l*c_l*c_l*GuoF[15]-2.00000000000000000*c_l*c_l*c_l*c_l*GuoF[16]-2.00000000000000000*c_l*c_l*c_l*c_l*GuoF[17]-2.00000000000000000*c_l*c_l*c_l*c_l*GuoF[18];

F_hat[10]*=(1-0.5*S[10]);
m_l[10]=m_l[10]-S[10]*(m_l[10]-meq[10])+dt*F_hat[10];
//=======================================

m_l[11]=+0.00000000000000000*c_l*c_l*f[ci][0]+0.00000000000000000*c_l*c_l*f[ci][1]+0.00000000000000000*c_l*c_l*f[ci][2]+1.00000000000000000*c_l*c_l*f[ci][3]+1.00000000000000000*c_l*c_l*f[ci][4]-1.00000000000000000*c_l*c_l*f[ci][5]-1.00000000000000000*c_l*c_l*f[ci][6]+1.00000000000000000*c_l*c_l*f[ci][7]+1.00000000000000000*c_l*c_l*f[ci][8]+1.00000000000000000*c_l*c_l*f[ci][9]+1.00000000000000000*c_l*c_l*f[ci][10]-1.00000000000000000*c_l*c_l*f[ci][11]-1.00000000000000000*c_l*c_l*f[ci][12]-1.00000000000000000*c_l*c_l*f[ci][13]-1.00000000000000000*c_l*c_l*f[ci][14]+0.00000000000000000*c_l*c_l*f[ci][15]+0.00000000000000000*c_l*c_l*f[ci][16]+0.00000000000000000*c_l*c_l*f[ci][17]+0.00000000000000000*c_l*c_l*f[ci][18];

F_hat[11]=+0.00000000000000000*c_l*c_l*GuoF[0]+0.00000000000000000*c_l*c_l*GuoF[1]+0.00000000000000000*c_l*c_l*GuoF[2]+1.00000000000000000*c_l*c_l*GuoF[3]+1.00000000000000000*c_l*c_l*GuoF[4]-1.00000000000000000*c_l*c_l*GuoF[5]-1.00000000000000000*c_l*c_l*GuoF[6]+1.00000000000000000*c_l*c_l*GuoF[7]+1.00000000000000000*c_l*c_l*GuoF[8]+1.00000000000000000*c_l*c_l*GuoF[9]+1.00000000000000000*c_l*c_l*GuoF[10]-1.00000000000000000*c_l*c_l*GuoF[11]-1.00000000000000000*c_l*c_l*GuoF[12]-1.00000000000000000*c_l*c_l*GuoF[13]-1.00000000000000000*c_l*c_l*GuoF[14]+0.00000000000000000*c_l*c_l*GuoF[15]+0.00000000000000000*c_l*c_l*GuoF[16]+0.00000000000000000*c_l*c_l*GuoF[17]+0.00000000000000000*c_l*c_l*GuoF[18];

F_hat[11]*=(1-0.5*S[11]);
m_l[11]=m_l[11]-S[11]*(m_l[11]-meq[11])+dt*F_hat[11];
//=======================================

m_l[12]=+0.00000000000000000*c_l*c_l*c_l*c_l*f[ci][0]+0.00000000000000000*c_l*c_l*c_l*c_l*f[ci][1]+0.00000000000000000*c_l*c_l*c_l*c_l*f[ci][2]-1.00000000000000000*c_l*c_l*c_l*c_l*f[ci][3]-1.00000000000000000*c_l*c_l*c_l*c_l*f[ci][4]+1.00000000000000000*c_l*c_l*c_l*c_l*f[ci][5]+1.00000000000000000*c_l*c_l*c_l*c_l*f[ci][6]+1.00000000000000000*c_l*c_l*c_l*c_l*f[ci][7]+1.00000000000000000*c_l*c_l*c_l*c_l*f[ci][8]+1.00000000000000000*c_l*c_l*c_l*c_l*f[ci][9]+1.00000000000000000*c_l*c_l*c_l*c_l*f[ci][10]-1.00000000000000000*c_l*c_l*c_l*c_l*f[ci][11]-1.00000000000000000*c_l*c_l*c_l*c_l*f[ci][12]-1.00000000000000000*c_l*c_l*c_l*c_l*f[ci][13]-1.00000000000000000*c_l*c_l*c_l*c_l*f[ci][14]+0.00000000000000000*c_l*c_l*c_l*c_l*f[ci][15]+0.00000000000000000*c_l*c_l*c_l*c_l*f[ci][16]+0.00000000000000000*c_l*c_l*c_l*c_l*f[ci][17]+0.00000000000000000*c_l*c_l*c_l*c_l*f[ci][18];

F_hat[12]=+0.00000000000000000*c_l*c_l*c_l*c_l*GuoF[0]+0.00000000000000000*c_l*c_l*c_l*c_l*GuoF[1]+0.00000000000000000*c_l*c_l*c_l*c_l*GuoF[2]-1.00000000000000000*c_l*c_l*c_l*c_l*GuoF[3]-1.00000000000000000*c_l*c_l*c_l*c_l*GuoF[4]+1.00000000000000000*c_l*c_l*c_l*c_l*GuoF[5]+1.00000000000000000*c_l*c_l*c_l*c_l*GuoF[6]+1.00000000000000000*c_l*c_l*c_l*c_l*GuoF[7]+1.00000000000000000*c_l*c_l*c_l*c_l*GuoF[8]+1.00000000000000000*c_l*c_l*c_l*c_l*GuoF[9]+1.00000000000000000*c_l*c_l*c_l*c_l*GuoF[10]-1.00000000000000000*c_l*c_l*c_l*c_l*GuoF[11]-1.00000000000000000*c_l*c_l*c_l*c_l*GuoF[12]-1.00000000000000000*c_l*c_l*c_l*c_l*GuoF[13]-1.00000000000000000*c_l*c_l*c_l*c_l*GuoF[14]+0.00000000000000000*c_l*c_l*c_l*c_l*GuoF[15]+0.00000000000000000*c_l*c_l*c_l*c_l*GuoF[16]+0.00000000000000000*c_l*c_l*c_l*c_l*GuoF[17]+0.00000000000000000*c_l*c_l*c_l*c_l*GuoF[18];

F_hat[12]*=(1-0.5*S[12]);
m_l[12]=m_l[12]-S[12]*(m_l[12]-meq[12])+dt*F_hat[12];
//=======================================

m_l[13]=+0.00000000000000000*c_l*c_l*f[ci][0]+0.00000000000000000*c_l*c_l*f[ci][1]+0.00000000000000000*c_l*c_l*f[ci][2]+0.00000000000000000*c_l*c_l*f[ci][3]+0.00000000000000000*c_l*c_l*f[ci][4]+0.00000000000000000*c_l*c_l*f[ci][5]+0.00000000000000000*c_l*c_l*f[ci][6]+1.00000000000000000*c_l*c_l*f[ci][7]+1.00000000000000000*c_l*c_l*f[ci][8]-1.00000000000000000*c_l*c_l*f[ci][9]-1.00000000000000000*c_l*c_l*f[ci][10]+0.00000000000000000*c_l*c_l*f[ci][11]+0.00000000000000000*c_l*c_l*f[ci][12]+0.00000000000000000*c_l*c_l*f[ci][13]+0.00000000000000000*c_l*c_l*f[ci][14]+0.00000000000000000*c_l*c_l*f[ci][15]+0.00000000000000000*c_l*c_l*f[ci][16]+0.00000000000000000*c_l*c_l*f[ci][17]+0.00000000000000000*c_l*c_l*f[ci][18];

F_hat[13]=+0.00000000000000000*c_l*c_l*GuoF[0]+0.00000000000000000*c_l*c_l*GuoF[1]+0.00000000000000000*c_l*c_l*GuoF[2]+0.00000000000000000*c_l*c_l*GuoF[3]+0.00000000000000000*c_l*c_l*GuoF[4]+0.00000000000000000*c_l*c_l*GuoF[5]+0.00000000000000000*c_l*c_l*GuoF[6]+1.00000000000000000*c_l*c_l*GuoF[7]+1.00000000000000000*c_l*c_l*GuoF[8]-1.00000000000000000*c_l*c_l*GuoF[9]-1.00000000000000000*c_l*c_l*GuoF[10]+0.00000000000000000*c_l*c_l*GuoF[11]+0.00000000000000000*c_l*c_l*GuoF[12]+0.00000000000000000*c_l*c_l*GuoF[13]+0.00000000000000000*c_l*c_l*GuoF[14]+0.00000000000000000*c_l*c_l*GuoF[15]+0.00000000000000000*c_l*c_l*GuoF[16]+0.00000000000000000*c_l*c_l*GuoF[17]+0.00000000000000000*c_l*c_l*GuoF[18];

F_hat[13]*=(1-0.5*S[13]);
m_l[13]=m_l[13]-S[13]*(m_l[13]-meq[13])+dt*F_hat[13];
//=======================================

m_l[14]=+0.00000000000000000*c_l*c_l*f[ci][0]+0.00000000000000000*c_l*c_l*f[ci][1]+0.00000000000000000*c_l*c_l*f[ci][2]+0.00000000000000000*c_l*c_l*f[ci][3]+0.00000000000000000*c_l*c_l*f[ci][4]+0.00000000000000000*c_l*c_l*f[ci][5]+0.00000000000000000*c_l*c_l*f[ci][6]+0.00000000000000000*c_l*c_l*f[ci][7]+0.00000000000000000*c_l*c_l*f[ci][8]+0.00000000000000000*c_l*c_l*f[ci][9]+0.00000000000000000*c_l*c_l*f[ci][10]+0.00000000000000000*c_l*c_l*f[ci][11]+0.00000000000000000*c_l*c_l*f[ci][12]+0.00000000000000000*c_l*c_l*f[ci][13]+0.00000000000000000*c_l*c_l*f[ci][14]+1.00000000000000000*c_l*c_l*f[ci][15]+1.00000000000000000*c_l*c_l*f[ci][16]-1.00000000000000000*c_l*c_l*f[ci][17]-1.00000000000000000*c_l*c_l*f[ci][18];

F_hat[14]=+0.00000000000000000*c_l*c_l*GuoF[0]+0.00000000000000000*c_l*c_l*GuoF[1]+0.00000000000000000*c_l*c_l*GuoF[2]+0.00000000000000000*c_l*c_l*GuoF[3]+0.00000000000000000*c_l*c_l*GuoF[4]+0.00000000000000000*c_l*c_l*GuoF[5]+0.00000000000000000*c_l*c_l*GuoF[6]+0.00000000000000000*c_l*c_l*GuoF[7]+0.00000000000000000*c_l*c_l*GuoF[8]+0.00000000000000000*c_l*c_l*GuoF[9]+0.00000000000000000*c_l*c_l*GuoF[10]+0.00000000000000000*c_l*c_l*GuoF[11]+0.00000000000000000*c_l*c_l*GuoF[12]+0.00000000000000000*c_l*c_l*GuoF[13]+0.00000000000000000*c_l*c_l*GuoF[14]+1.00000000000000000*c_l*c_l*GuoF[15]+1.00000000000000000*c_l*c_l*GuoF[16]-1.00000000000000000*c_l*c_l*GuoF[17]-1.00000000000000000*c_l*c_l*GuoF[18];

F_hat[14]*=(1-0.5*S[14]);
m_l[14]=m_l[14]-S[14]*(m_l[14]-meq[14])+dt*F_hat[14];
//=======================================

m_l[15]=+0.00000000000000000*c_l*c_l*f[ci][0]+0.00000000000000000*c_l*c_l*f[ci][1]+0.00000000000000000*c_l*c_l*f[ci][2]+0.00000000000000000*c_l*c_l*f[ci][3]+0.00000000000000000*c_l*c_l*f[ci][4]+0.00000000000000000*c_l*c_l*f[ci][5]+0.00000000000000000*c_l*c_l*f[ci][6]+0.00000000000000000*c_l*c_l*f[ci][7]+0.00000000000000000*c_l*c_l*f[ci][8]+0.00000000000000000*c_l*c_l*f[ci][9]+0.00000000000000000*c_l*c_l*f[ci][10]+1.00000000000000000*c_l*c_l*f[ci][11]+1.00000000000000000*c_l*c_l*f[ci][12]-1.00000000000000000*c_l*c_l*f[ci][13]-1.00000000000000000*c_l*c_l*f[ci][14]+0.00000000000000000*c_l*c_l*f[ci][15]+0.00000000000000000*c_l*c_l*f[ci][16]+0.00000000000000000*c_l*c_l*f[ci][17]+0.00000000000000000*c_l*c_l*f[ci][18];

F_hat[15]=+0.00000000000000000*c_l*c_l*GuoF[0]+0.00000000000000000*c_l*c_l*GuoF[1]+0.00000000000000000*c_l*c_l*GuoF[2]+0.00000000000000000*c_l*c_l*GuoF[3]+0.00000000000000000*c_l*c_l*GuoF[4]+0.00000000000000000*c_l*c_l*GuoF[5]+0.00000000000000000*c_l*c_l*GuoF[6]+0.00000000000000000*c_l*c_l*GuoF[7]+0.00000000000000000*c_l*c_l*GuoF[8]+0.00000000000000000*c_l*c_l*GuoF[9]+0.00000000000000000*c_l*c_l*GuoF[10]+1.00000000000000000*c_l*c_l*GuoF[11]+1.00000000000000000*c_l*c_l*GuoF[12]-1.00000000000000000*c_l*c_l*GuoF[13]-1.00000000000000000*c_l*c_l*GuoF[14]+0.00000000000000000*c_l*c_l*GuoF[15]+0.00000000000000000*c_l*c_l*GuoF[16]+0.00000000000000000*c_l*c_l*GuoF[17]+0.00000000000000000*c_l*c_l*GuoF[18];

F_hat[15]*=(1-0.5*S[15]);
m_l[15]=m_l[15]-S[15]*(m_l[15]-meq[15])+dt*F_hat[15];
//=======================================

m_l[16]=+0.00000000000000000*c_l*c_l*c_l*f[ci][0]+0.00000000000000000*c_l*c_l*c_l*f[ci][1]+0.00000000000000000*c_l*c_l*c_l*f[ci][2]+0.00000000000000000*c_l*c_l*c_l*f[ci][3]+0.00000000000000000*c_l*c_l*c_l*f[ci][4]+0.00000000000000000*c_l*c_l*c_l*f[ci][5]+0.00000000000000000*c_l*c_l*c_l*f[ci][6]+1.00000000000000000*c_l*c_l*c_l*f[ci][7]-1.00000000000000000*c_l*c_l*c_l*f[ci][8]+1.00000000000000000*c_l*c_l*c_l*f[ci][9]-1.00000000000000000*c_l*c_l*c_l*f[ci][10]-1.00000000000000000*c_l*c_l*c_l*f[ci][11]+1.00000000000000000*c_l*c_l*c_l*f[ci][12]-1.00000000000000000*c_l*c_l*c_l*f[ci][13]+1.00000000000000000*c_l*c_l*c_l*f[ci][14]+0.00000000000000000*c_l*c_l*c_l*f[ci][15]+0.00000000000000000*c_l*c_l*c_l*f[ci][16]+0.00000000000000000*c_l*c_l*c_l*f[ci][17]+0.00000000000000000*c_l*c_l*c_l*f[ci][18];

F_hat[16]=+0.00000000000000000*c_l*c_l*c_l*GuoF[0]+0.00000000000000000*c_l*c_l*c_l*GuoF[1]+0.00000000000000000*c_l*c_l*c_l*GuoF[2]+0.00000000000000000*c_l*c_l*c_l*GuoF[3]+0.00000000000000000*c_l*c_l*c_l*GuoF[4]+0.00000000000000000*c_l*c_l*c_l*GuoF[5]+0.00000000000000000*c_l*c_l*c_l*GuoF[6]+1.00000000000000000*c_l*c_l*c_l*GuoF[7]-1.00000000000000000*c_l*c_l*c_l*GuoF[8]+1.00000000000000000*c_l*c_l*c_l*GuoF[9]-1.00000000000000000*c_l*c_l*c_l*GuoF[10]-1.00000000000000000*c_l*c_l*c_l*GuoF[11]+1.00000000000000000*c_l*c_l*c_l*GuoF[12]-1.00000000000000000*c_l*c_l*c_l*GuoF[13]+1.00000000000000000*c_l*c_l*c_l*GuoF[14]+0.00000000000000000*c_l*c_l*c_l*GuoF[15]+0.00000000000000000*c_l*c_l*c_l*GuoF[16]+0.00000000000000000*c_l*c_l*c_l*GuoF[17]+0.00000000000000000*c_l*c_l*c_l*GuoF[18];

F_hat[16]*=(1-0.5*S[16]);
m_l[16]=m_l[16]-S[16]*(m_l[16]-meq[16])+dt*F_hat[16];
//=======================================

m_l[17]=+0.00000000000000000*c_l*c_l*c_l*f[ci][0]+0.00000000000000000*c_l*c_l*c_l*f[ci][1]+0.00000000000000000*c_l*c_l*c_l*f[ci][2]+0.00000000000000000*c_l*c_l*c_l*f[ci][3]+0.00000000000000000*c_l*c_l*c_l*f[ci][4]+0.00000000000000000*c_l*c_l*c_l*f[ci][5]+0.00000000000000000*c_l*c_l*c_l*f[ci][6]-1.00000000000000000*c_l*c_l*c_l*f[ci][7]+1.00000000000000000*c_l*c_l*c_l*f[ci][8]+1.00000000000000000*c_l*c_l*c_l*f[ci][9]-1.00000000000000000*c_l*c_l*c_l*f[ci][10]+0.00000000000000000*c_l*c_l*c_l*f[ci][11]+0.00000000000000000*c_l*c_l*c_l*f[ci][12]+0.00000000000000000*c_l*c_l*c_l*f[ci][13]+0.00000000000000000*c_l*c_l*c_l*f[ci][14]+1.00000000000000000*c_l*c_l*c_l*f[ci][15]-1.00000000000000000*c_l*c_l*c_l*f[ci][16]+1.00000000000000000*c_l*c_l*c_l*f[ci][17]-1.00000000000000000*c_l*c_l*c_l*f[ci][18];

F_hat[17]=+0.00000000000000000*c_l*c_l*c_l*GuoF[0]+0.00000000000000000*c_l*c_l*c_l*GuoF[1]+0.00000000000000000*c_l*c_l*c_l*GuoF[2]+0.00000000000000000*c_l*c_l*c_l*GuoF[3]+0.00000000000000000*c_l*c_l*c_l*GuoF[4]+0.00000000000000000*c_l*c_l*c_l*GuoF[5]+0.00000000000000000*c_l*c_l*c_l*GuoF[6]-1.00000000000000000*c_l*c_l*c_l*GuoF[7]+1.00000000000000000*c_l*c_l*c_l*GuoF[8]+1.00000000000000000*c_l*c_l*c_l*GuoF[9]-1.00000000000000000*c_l*c_l*c_l*GuoF[10]+0.00000000000000000*c_l*c_l*c_l*GuoF[11]+0.00000000000000000*c_l*c_l*c_l*GuoF[12]+0.00000000000000000*c_l*c_l*c_l*GuoF[13]+0.00000000000000000*c_l*c_l*c_l*GuoF[14]+1.00000000000000000*c_l*c_l*c_l*GuoF[15]-1.00000000000000000*c_l*c_l*c_l*GuoF[16]+1.00000000000000000*c_l*c_l*c_l*GuoF[17]-1.00000000000000000*c_l*c_l*c_l*GuoF[18];

F_hat[17]*=(1-0.5*S[17]);
m_l[17]=m_l[17]-S[17]*(m_l[17]-meq[17])+dt*F_hat[17];
//=======================================

m_l[18]=+0.00000000000000000*c_l*c_l*c_l*f[ci][0]+0.00000000000000000*c_l*c_l*c_l*f[ci][1]+0.00000000000000000*c_l*c_l*c_l*f[ci][2]+0.00000000000000000*c_l*c_l*c_l*f[ci][3]+0.00000000000000000*c_l*c_l*c_l*f[ci][4]+0.00000000000000000*c_l*c_l*c_l*f[ci][5]+0.00000000000000000*c_l*c_l*c_l*f[ci][6]+0.00000000000000000*c_l*c_l*c_l*f[ci][7]+0.00000000000000000*c_l*c_l*c_l*f[ci][8]+0.00000000000000000*c_l*c_l*c_l*f[ci][9]+0.00000000000000000*c_l*c_l*c_l*f[ci][10]+1.00000000000000000*c_l*c_l*c_l*f[ci][11]-1.00000000000000000*c_l*c_l*c_l*f[ci][12]-1.00000000000000000*c_l*c_l*c_l*f[ci][13]+1.00000000000000000*c_l*c_l*c_l*f[ci][14]-1.00000000000000000*c_l*c_l*c_l*f[ci][15]+1.00000000000000000*c_l*c_l*c_l*f[ci][16]+1.00000000000000000*c_l*c_l*c_l*f[ci][17]-1.00000000000000000*c_l*c_l*c_l*f[ci][18];

F_hat[18]=+0.00000000000000000*c_l*c_l*c_l*GuoF[0]+0.00000000000000000*c_l*c_l*c_l*GuoF[1]+0.00000000000000000*c_l*c_l*c_l*GuoF[2]+0.00000000000000000*c_l*c_l*c_l*GuoF[3]+0.00000000000000000*c_l*c_l*c_l*GuoF[4]+0.00000000000000000*c_l*c_l*c_l*GuoF[5]+0.00000000000000000*c_l*c_l*c_l*GuoF[6]+0.00000000000000000*c_l*c_l*c_l*GuoF[7]+0.00000000000000000*c_l*c_l*c_l*GuoF[8]+0.00000000000000000*c_l*c_l*c_l*GuoF[9]+0.00000000000000000*c_l*c_l*c_l*GuoF[10]+1.00000000000000000*c_l*c_l*c_l*GuoF[11]-1.00000000000000000*c_l*c_l*c_l*GuoF[12]-1.00000000000000000*c_l*c_l*c_l*GuoF[13]+1.00000000000000000*c_l*c_l*c_l*GuoF[14]-1.00000000000000000*c_l*c_l*c_l*GuoF[15]+1.00000000000000000*c_l*c_l*c_l*GuoF[16]+1.00000000000000000*c_l*c_l*c_l*GuoF[17]-1.00000000000000000*c_l*c_l*c_l*GuoF[18];

F_hat[18]*=(1-0.5*S[18]);
m_l[18]=m_l[18]-S[18]*(m_l[18]-meq[18])+dt*F_hat[18];
//=======================================

//for (int isl=0;isl<19;isl++)
//	if (m_l[isl]!=m_l[isl])
//		cout<<isl<<"	"<<n<<"		"<<rank<<"	"<<s_v<<"		"<<s_other<<endl;

//==========================
m_inv_l[0]=+((double)0X1.5555555555555P-2)*1.0*m_l[0]+((double)-0X1P-1)*1.0/(c_l*c_l)*m_l[1]+((double)0X1.5555555555555P-3)*1.0/(c_l*c_l*c_l*c_l)*m_l[2]+((double)0X0P+0)*1.0/c_l*m_l[3]+((double)0X0P+0)*1.0/(c_l*c_l*c_l)*m_l[4]+((double)0X0P+0)*1.0/(c_l)*m_l[5]+((double)0X0P+0)*1.0/(c_l*c_l*c_l)*m_l[6]+((double)0X0P+0)*1.0/c_l*m_l[7]+((double)0X0P+0)*1.0/(c_l*c_l*c_l)*m_l[8]+((double)0X0P+0)*1.0/(c_l*c_l)*m_l[9]+((double)0X0P+0)*1.0/(c_l*c_l*c_l*c_l)*m_l[10]+((double)0X0P+0)*1.0/(c_l*c_l)*m_l[11]+((double)0X0P+0)*1.0/(c_l*c_l*c_l*c_l)*m_l[12]+((double)0X0P+0)*1.0/(c_l*c_l)*m_l[13]+((double)0X0P+0)*1.0/(c_l*c_l)*m_l[14]+((double)0X0P+0)*1.0/(c_l*c_l)*m_l[15]+((double)0X0P+0)*1.0/(c_l*c_l*c_l)*m_l[16]+((double)0X0P+0)*1.0/(c_l*c_l*c_l)*m_l[17]+((double)0X0P+0)*1.0/(c_l*c_l*c_l)*m_l[18];

m_inv_l[1]=+((double)0X1.C71C71C71C71CP-5)*1.0*m_l[0]+((double)0X0P+0)*1.0/(c_l*c_l)*m_l[1]+((double)-0X1.C71C71C71C71EP-5)*1.0/(c_l*c_l*c_l*c_l)*m_l[2]+((double)0X1.5555555555555P-3)*1.0/c_l*m_l[3]+((double)-0X1.5555555555556P-3)*1.0/(c_l*c_l*c_l)*m_l[4]+((double)0X0P+0)*1.0/(c_l)*m_l[5]+((double)0X0P+0)*1.0/(c_l*c_l*c_l)*m_l[6]+((double)0X0P+0)*1.0/c_l*m_l[7]+((double)0X0P+0)*1.0/(c_l*c_l*c_l)*m_l[8]+((double)0X1.5555555555556P-4)*1.0/(c_l*c_l)*m_l[9]+((double)-0X1.5555555555554P-4)*1.0/(c_l*c_l*c_l*c_l)*m_l[10]+((double)0X0P+0)*1.0/(c_l*c_l)*m_l[11]+((double)0X0P+0)*1.0/(c_l*c_l*c_l*c_l)*m_l[12]+((double)0X0P+0)*1.0/(c_l*c_l)*m_l[13]+((double)0X0P+0)*1.0/(c_l*c_l)*m_l[14]+((double)0X0P+0)*1.0/(c_l*c_l)*m_l[15]+((double)0X0P+0)*1.0/(c_l*c_l*c_l)*m_l[16]+((double)0X0P+0)*1.0/(c_l*c_l*c_l)*m_l[17]+((double)0X0P+0)*1.0/(c_l*c_l*c_l)*m_l[18];

m_inv_l[2]=+((double)0X1.C71C71C71C71DP-5)*1.0*m_l[0]+((double)-0X1P-56)*1.0/(c_l*c_l)*m_l[1]+((double)-0X1.C71C71C71C71DP-5)*1.0/(c_l*c_l*c_l*c_l)*m_l[2]+((double)-0X1.5555555555555P-3)*1.0/c_l*m_l[3]+((double)0X1.5555555555556P-3)*1.0/(c_l*c_l*c_l)*m_l[4]+((double)0X0P+0)*1.0/(c_l)*m_l[5]+((double)0X0P+0)*1.0/(c_l*c_l*c_l)*m_l[6]+((double)0X0P+0)*1.0/c_l*m_l[7]+((double)0X0P+0)*1.0/(c_l*c_l*c_l)*m_l[8]+((double)0X1.5555555555556P-4)*1.0/(c_l*c_l)*m_l[9]+((double)-0X1.5555555555554P-4)*1.0/(c_l*c_l*c_l*c_l)*m_l[10]+((double)0X0P+0)*1.0/(c_l*c_l)*m_l[11]+((double)0X0P+0)*1.0/(c_l*c_l*c_l*c_l)*m_l[12]+((double)0X0P+0)*1.0/(c_l*c_l)*m_l[13]+((double)0X0P+0)*1.0/(c_l*c_l)*m_l[14]+((double)0X0P+0)*1.0/(c_l*c_l)*m_l[15]+((double)0X0P+0)*1.0/(c_l*c_l*c_l)*m_l[16]+((double)0X0P+0)*1.0/(c_l*c_l*c_l)*m_l[17]+((double)0X0P+0)*1.0/(c_l*c_l*c_l)*m_l[18];

m_inv_l[3]=+((double)0X1.C71C71C71C71DP-5)*1.0*m_l[0]+((double)0X1P-56)*1.0/(c_l*c_l)*m_l[1]+((double)-0X1.C71C71C71C71EP-5)*1.0/(c_l*c_l*c_l*c_l)*m_l[2]+((double)0X0P+0)*1.0/c_l*m_l[3]+((double)0X0P+0)*1.0/(c_l*c_l*c_l)*m_l[4]+((double)0X1.5555555555554P-3)*1.0/(c_l)*m_l[5]+((double)-0X1.5555555555556P-3)*1.0/(c_l*c_l*c_l)*m_l[6]+((double)0X0P+0)*1.0/c_l*m_l[7]+((double)0X0P+0)*1.0/(c_l*c_l*c_l)*m_l[8]+((double)-0X1.5555555555555P-5)*1.0/(c_l*c_l)*m_l[9]+((double)0X1.5555555555555P-5)*1.0/(c_l*c_l*c_l*c_l)*m_l[10]+((double)0X1.FFFFFFFFFFFFEP-4)*1.0/(c_l*c_l)*m_l[11]+((double)-0X1.0000000000001P-3)*1.0/(c_l*c_l*c_l*c_l)*m_l[12]+((double)0X0P+0)*1.0/(c_l*c_l)*m_l[13]+((double)0X0P+0)*1.0/(c_l*c_l)*m_l[14]+((double)0X0P+0)*1.0/(c_l*c_l)*m_l[15]+((double)0X0P+0)*1.0/(c_l*c_l*c_l)*m_l[16]+((double)0X1P-56)*1.0/(c_l*c_l*c_l)*m_l[17]+((double)0X0P+0)*1.0/(c_l*c_l*c_l)*m_l[18];

m_inv_l[4]=+((double)0X1.C71C71C71C71DP-5)*1.0*m_l[0]+((double)0X0P+0)*1.0/(c_l*c_l)*m_l[1]+((double)-0X1.C71C71C71C71AP-5)*1.0/(c_l*c_l*c_l*c_l)*m_l[2]+((double)0X0P+0)*1.0/c_l*m_l[3]+((double)0X0P+0)*1.0/(c_l*c_l*c_l)*m_l[4]+((double)-0X1.5555555555555P-3)*1.0/(c_l)*m_l[5]+((double)0X1.5555555555556P-3)*1.0/(c_l*c_l*c_l)*m_l[6]+((double)0X0P+0)*1.0/c_l*m_l[7]+((double)0X0P+0)*1.0/(c_l*c_l*c_l)*m_l[8]+((double)-0X1.5555555555555P-5)*1.0/(c_l*c_l)*m_l[9]+((double)0X1.5555555555555P-5)*1.0/(c_l*c_l*c_l*c_l)*m_l[10]+((double)0X1.FFFFFFFFFFFFEP-4)*1.0/(c_l*c_l)*m_l[11]+((double)-0X1.0000000000001P-3)*1.0/(c_l*c_l*c_l*c_l)*m_l[12]+((double)0X0P+0)*1.0/(c_l*c_l)*m_l[13]+((double)0X0P+0)*1.0/(c_l*c_l)*m_l[14]+((double)0X0P+0)*1.0/(c_l*c_l)*m_l[15]+((double)0X0P+0)*1.0/(c_l*c_l*c_l)*m_l[16]+((double)-0X1P-56)*1.0/(c_l*c_l*c_l)*m_l[17]+((double)0X0P+0)*1.0/(c_l*c_l*c_l)*m_l[18];

m_inv_l[5]=+((double)0X1.C71C71C71C718P-5)*1.0*m_l[0]+((double)-0X1.8P-56)*1.0/(c_l*c_l)*m_l[1]+((double)-0X1.C71C71C71C71DP-5)*1.0/(c_l*c_l*c_l*c_l)*m_l[2]+((double)0X0P+0)*1.0/c_l*m_l[3]+((double)0X0P+0)*1.0/(c_l*c_l*c_l)*m_l[4]+((double)0X0P+0)*1.0/(c_l)*m_l[5]+((double)0X0P+0)*1.0/(c_l*c_l*c_l)*m_l[6]+((double)0X1.5555555555555P-3)*1.0/c_l*m_l[7]+((double)-0X1.5555555555556P-3)*1.0/(c_l*c_l*c_l)*m_l[8]+((double)-0X1.5555555555558P-5)*1.0/(c_l*c_l)*m_l[9]+((double)0X1.5555555555552P-5)*1.0/(c_l*c_l*c_l*c_l)*m_l[10]+((double)-0X1.FFFFFFFFFFFFDP-4)*1.0/(c_l*c_l)*m_l[11]+((double)0X1.0000000000002P-3)*1.0/(c_l*c_l*c_l*c_l)*m_l[12]+((double)0X0P+0)*1.0/(c_l*c_l)*m_l[13]+((double)0X0P+0)*1.0/(c_l*c_l)*m_l[14]+((double)0X0P+0)*1.0/(c_l*c_l)*m_l[15]+((double)0X0P+0)*1.0/(c_l*c_l*c_l)*m_l[16]+((double)0X0P+0)*1.0/(c_l*c_l*c_l)*m_l[17]+((double)0X0P+0)*1.0/(c_l*c_l*c_l)*m_l[18];

m_inv_l[6]=+((double)0X1.C71C71C71C71AP-5)*1.0*m_l[0]+((double)0X0P+0)*1.0/(c_l*c_l)*m_l[1]+((double)-0X1.C71C71C71C71DP-5)*1.0/(c_l*c_l*c_l*c_l)*m_l[2]+((double)0X0P+0)*1.0/c_l*m_l[3]+((double)0X0P+0)*1.0/(c_l*c_l*c_l)*m_l[4]+((double)-0X1.5555555555554P-58)*1.0/(c_l)*m_l[5]+((double)-0X1.5555555555554P-59)*1.0/(c_l*c_l*c_l)*m_l[6]+((double)-0X1.5555555555555P-3)*1.0/c_l*m_l[7]+((double)0X1.5555555555555P-3)*1.0/(c_l*c_l*c_l)*m_l[8]+((double)-0X1.5555555555556P-5)*1.0/(c_l*c_l)*m_l[9]+((double)0X1.5555555555554P-5)*1.0/(c_l*c_l*c_l*c_l)*m_l[10]+((double)-0X1.FFFFFFFFFFFFEP-4)*1.0/(c_l*c_l)*m_l[11]+((double)0X1P-3)*1.0/(c_l*c_l*c_l*c_l)*m_l[12]+((double)0X0P+0)*1.0/(c_l*c_l)*m_l[13]+((double)0X0P+0)*1.0/(c_l*c_l)*m_l[14]+((double)0X0P+0)*1.0/(c_l*c_l)*m_l[15]+((double)0X0P+0)*1.0/(c_l*c_l*c_l)*m_l[16]+((double)0X0P+0)*1.0/(c_l*c_l*c_l)*m_l[17]+((double)-0X1P-55)*1.0/(c_l*c_l*c_l)*m_l[18];

m_inv_l[7]=+((double)0X1.C71C71C71C71BP-6)*1.0*m_l[0]+((double)0X1.5555555555555P-5)*1.0/(c_l*c_l)*m_l[1]+((double)0X1.C71C71C71C71BP-7)*1.0/(c_l*c_l*c_l*c_l)*m_l[2]+((double)0X1.5555555555555P-4)*1.0/c_l*m_l[3]+((double)0X1.5555555555555P-5)*1.0/(c_l*c_l*c_l)*m_l[4]+((double)0X1.5555555555555P-4)*1.0/(c_l)*m_l[5]+((double)0X1.5555555555555P-5)*1.0/(c_l*c_l*c_l)*m_l[6]+((double)0X0P+0)*1.0/c_l*m_l[7]+((double)0X0P+0)*1.0/(c_l*c_l*c_l)*m_l[8]+((double)0X1.5555555555555P-6)*1.0/(c_l*c_l)*m_l[9]+((double)0X1.5555555555555P-6)*1.0/(c_l*c_l*c_l*c_l)*m_l[10]+((double)0X1.0000000000001P-4)*1.0/(c_l*c_l)*m_l[11]+((double)0X1.0000000000001P-4)*1.0/(c_l*c_l*c_l*c_l)*m_l[12]+((double)0X1P-2)*1.0/(c_l*c_l)*m_l[13]+((double)0X0P+0)*1.0/(c_l*c_l)*m_l[14]+((double)0X0P+0)*1.0/(c_l*c_l)*m_l[15]+((double)0X1P-3)*1.0/(c_l*c_l*c_l)*m_l[16]+((double)-0X1P-3)*1.0/(c_l*c_l*c_l)*m_l[17]+((double)0X0P+0)*1.0/(c_l*c_l*c_l)*m_l[18];

m_inv_l[8]=+((double)0X1.C71C71C71C71BP-6)*1.0*m_l[0]+((double)0X1.5555555555555P-5)*1.0/(c_l*c_l)*m_l[1]+((double)0X1.C71C71C71C71BP-7)*1.0/(c_l*c_l*c_l*c_l)*m_l[2]+((double)-0X1.5555555555555P-4)*1.0/c_l*m_l[3]+((double)-0X1.5555555555555P-5)*1.0/(c_l*c_l*c_l)*m_l[4]+((double)-0X1.5555555555555P-4)*1.0/(c_l)*m_l[5]+((double)-0X1.5555555555555P-5)*1.0/(c_l*c_l*c_l)*m_l[6]+((double)0X0P+0)*1.0/c_l*m_l[7]+((double)0X0P+0)*1.0/(c_l*c_l*c_l)*m_l[8]+((double)0X1.5555555555555P-6)*1.0/(c_l*c_l)*m_l[9]+((double)0X1.5555555555555P-6)*1.0/(c_l*c_l*c_l*c_l)*m_l[10]+((double)0X1.0000000000001P-4)*1.0/(c_l*c_l)*m_l[11]+((double)0X1.0000000000001P-4)*1.0/(c_l*c_l*c_l*c_l)*m_l[12]+((double)0X1P-2)*1.0/(c_l*c_l)*m_l[13]+((double)0X0P+0)*1.0/(c_l*c_l)*m_l[14]+((double)0X0P+0)*1.0/(c_l*c_l)*m_l[15]+((double)-0X1P-3)*1.0/(c_l*c_l*c_l)*m_l[16]+((double)0X1P-3)*1.0/(c_l*c_l*c_l)*m_l[17]+((double)0X0P+0)*1.0/(c_l*c_l*c_l)*m_l[18];

m_inv_l[9]=+((double)0X1.C71C71C71C71BP-6)*1.0*m_l[0]+((double)0X1.5555555555555P-5)*1.0/(c_l*c_l)*m_l[1]+((double)0X1.C71C71C71C71BP-7)*1.0/(c_l*c_l*c_l*c_l)*m_l[2]+((double)0X1.5555555555555P-4)*1.0/c_l*m_l[3]+((double)0X1.5555555555555P-5)*1.0/(c_l*c_l*c_l)*m_l[4]+((double)-0X1.5555555555555P-4)*1.0/(c_l)*m_l[5]+((double)-0X1.5555555555555P-5)*1.0/(c_l*c_l*c_l)*m_l[6]+((double)0X0P+0)*1.0/c_l*m_l[7]+((double)0X0P+0)*1.0/(c_l*c_l*c_l)*m_l[8]+((double)0X1.5555555555555P-6)*1.0/(c_l*c_l)*m_l[9]+((double)0X1.5555555555555P-6)*1.0/(c_l*c_l*c_l*c_l)*m_l[10]+((double)0X1.0000000000001P-4)*1.0/(c_l*c_l)*m_l[11]+((double)0X1.0000000000001P-4)*1.0/(c_l*c_l*c_l*c_l)*m_l[12]+((double)-0X1P-2)*1.0/(c_l*c_l)*m_l[13]+((double)0X0P+0)*1.0/(c_l*c_l)*m_l[14]+((double)0X0P+0)*1.0/(c_l*c_l)*m_l[15]+((double)0X1P-3)*1.0/(c_l*c_l*c_l)*m_l[16]+((double)0X1P-3)*1.0/(c_l*c_l*c_l)*m_l[17]+((double)0X0P+0)*1.0/(c_l*c_l*c_l)*m_l[18];

m_inv_l[10]=+((double)0X1.C71C71C71C71BP-6)*1.0*m_l[0]+((double)0X1.5555555555555P-5)*1.0/(c_l*c_l)*m_l[1]+((double)0X1.C71C71C71C71BP-7)*1.0/(c_l*c_l*c_l*c_l)*m_l[2]+((double)-0X1.5555555555555P-4)*1.0/c_l*m_l[3]+((double)-0X1.5555555555555P-5)*1.0/(c_l*c_l*c_l)*m_l[4]+((double)0X1.5555555555555P-4)*1.0/(c_l)*m_l[5]+((double)0X1.5555555555555P-5)*1.0/(c_l*c_l*c_l)*m_l[6]+((double)0X0P+0)*1.0/c_l*m_l[7]+((double)0X0P+0)*1.0/(c_l*c_l*c_l)*m_l[8]+((double)0X1.5555555555555P-6)*1.0/(c_l*c_l)*m_l[9]+((double)0X1.5555555555555P-6)*1.0/(c_l*c_l*c_l*c_l)*m_l[10]+((double)0X1.0000000000001P-4)*1.0/(c_l*c_l)*m_l[11]+((double)0X1.0000000000001P-4)*1.0/(c_l*c_l*c_l*c_l)*m_l[12]+((double)-0X1P-2)*1.0/(c_l*c_l)*m_l[13]+((double)0X0P+0)*1.0/(c_l*c_l)*m_l[14]+((double)0X0P+0)*1.0/(c_l*c_l)*m_l[15]+((double)-0X1P-3)*1.0/(c_l*c_l*c_l)*m_l[16]+((double)-0X1P-3)*1.0/(c_l*c_l*c_l)*m_l[17]+((double)0X0P+0)*1.0/(c_l*c_l*c_l)*m_l[18];

m_inv_l[11]=+((double)0X1.C71C71C71C71DP-6)*1.0*m_l[0]+((double)0X1.5555555555555P-5)*1.0/(c_l*c_l)*m_l[1]+((double)0X1.C71C71C71C71DP-7)*1.0/(c_l*c_l*c_l*c_l)*m_l[2]+((double)0X1.5555555555555P-4)*1.0/c_l*m_l[3]+((double)0X1.5555555555555P-5)*1.0/(c_l*c_l*c_l)*m_l[4]+((double)0X0P+0)*1.0/(c_l)*m_l[5]+((double)0X0P+0)*1.0/(c_l*c_l*c_l)*m_l[6]+((double)0X1.5555555555555P-4)*1.0/c_l*m_l[7]+((double)0X1.5555555555555P-5)*1.0/(c_l*c_l*c_l)*m_l[8]+((double)0X1.5555555555555P-6)*1.0/(c_l*c_l)*m_l[9]+((double)0X1.5555555555555P-6)*1.0/(c_l*c_l*c_l*c_l)*m_l[10]+((double)-0X1.0000000000001P-4)*1.0/(c_l*c_l)*m_l[11]+((double)-0X1.0000000000001P-4)*1.0/(c_l*c_l*c_l*c_l)*m_l[12]+((double)0X0P+0)*1.0/(c_l*c_l)*m_l[13]+((double)0X0P+0)*1.0/(c_l*c_l)*m_l[14]+((double)0X1P-2)*1.0/(c_l*c_l)*m_l[15]+((double)-0X1P-3)*1.0/(c_l*c_l*c_l)*m_l[16]+((double)0X0P+0)*1.0/(c_l*c_l*c_l)*m_l[17]+((double)0X1P-3)*1.0/(c_l*c_l*c_l)*m_l[18];

m_inv_l[12]=+((double)0X1.C71C71C71C71DP-6)*1.0*m_l[0]+((double)0X1.5555555555555P-5)*1.0/(c_l*c_l)*m_l[1]+((double)0X1.C71C71C71C71DP-7)*1.0/(c_l*c_l*c_l*c_l)*m_l[2]+((double)-0X1.5555555555555P-4)*1.0/c_l*m_l[3]+((double)-0X1.5555555555555P-5)*1.0/(c_l*c_l*c_l)*m_l[4]+((double)0X0P+0)*1.0/(c_l)*m_l[5]+((double)0X0P+0)*1.0/(c_l*c_l*c_l)*m_l[6]+((double)-0X1.5555555555555P-4)*1.0/c_l*m_l[7]+((double)-0X1.5555555555555P-5)*1.0/(c_l*c_l*c_l)*m_l[8]+((double)0X1.5555555555555P-6)*1.0/(c_l*c_l)*m_l[9]+((double)0X1.5555555555555P-6)*1.0/(c_l*c_l*c_l*c_l)*m_l[10]+((double)-0X1.0000000000001P-4)*1.0/(c_l*c_l)*m_l[11]+((double)-0X1.0000000000001P-4)*1.0/(c_l*c_l*c_l*c_l)*m_l[12]+((double)0X0P+0)*1.0/(c_l*c_l)*m_l[13]+((double)0X0P+0)*1.0/(c_l*c_l)*m_l[14]+((double)0X1P-2)*1.0/(c_l*c_l)*m_l[15]+((double)0X1P-3)*1.0/(c_l*c_l*c_l)*m_l[16]+((double)0X0P+0)*1.0/(c_l*c_l*c_l)*m_l[17]+((double)-0X1P-3)*1.0/(c_l*c_l*c_l)*m_l[18];

m_inv_l[13]=+((double)0X1.C71C71C71C71DP-6)*1.0*m_l[0]+((double)0X1.5555555555555P-5)*1.0/(c_l*c_l)*m_l[1]+((double)0X1.C71C71C71C71DP-7)*1.0/(c_l*c_l*c_l*c_l)*m_l[2]+((double)0X1.5555555555555P-4)*1.0/c_l*m_l[3]+((double)0X1.5555555555555P-5)*1.0/(c_l*c_l*c_l)*m_l[4]+((double)0X0P+0)*1.0/(c_l)*m_l[5]+((double)0X0P+0)*1.0/(c_l*c_l*c_l)*m_l[6]+((double)-0X1.5555555555555P-4)*1.0/c_l*m_l[7]+((double)-0X1.5555555555555P-5)*1.0/(c_l*c_l*c_l)*m_l[8]+((double)0X1.5555555555555P-6)*1.0/(c_l*c_l)*m_l[9]+((double)0X1.5555555555555P-6)*1.0/(c_l*c_l*c_l*c_l)*m_l[10]+((double)-0X1.0000000000001P-4)*1.0/(c_l*c_l)*m_l[11]+((double)-0X1.0000000000001P-4)*1.0/(c_l*c_l*c_l*c_l)*m_l[12]+((double)0X0P+0)*1.0/(c_l*c_l)*m_l[13]+((double)0X0P+0)*1.0/(c_l*c_l)*m_l[14]+((double)-0X1P-2)*1.0/(c_l*c_l)*m_l[15]+((double)-0X1P-3)*1.0/(c_l*c_l*c_l)*m_l[16]+((double)0X0P+0)*1.0/(c_l*c_l*c_l)*m_l[17]+((double)-0X1P-3)*1.0/(c_l*c_l*c_l)*m_l[18];

m_inv_l[14]=+((double)0X1.C71C71C71C71DP-6)*1.0*m_l[0]+((double)0X1.5555555555555P-5)*1.0/(c_l*c_l)*m_l[1]+((double)0X1.C71C71C71C71DP-7)*1.0/(c_l*c_l*c_l*c_l)*m_l[2]+((double)-0X1.5555555555555P-4)*1.0/c_l*m_l[3]+((double)-0X1.5555555555555P-5)*1.0/(c_l*c_l*c_l)*m_l[4]+((double)0X0P+0)*1.0/(c_l)*m_l[5]+((double)0X0P+0)*1.0/(c_l*c_l*c_l)*m_l[6]+((double)0X1.5555555555555P-4)*1.0/c_l*m_l[7]+((double)0X1.5555555555555P-5)*1.0/(c_l*c_l*c_l)*m_l[8]+((double)0X1.5555555555555P-6)*1.0/(c_l*c_l)*m_l[9]+((double)0X1.5555555555555P-6)*1.0/(c_l*c_l*c_l*c_l)*m_l[10]+((double)-0X1.0000000000001P-4)*1.0/(c_l*c_l)*m_l[11]+((double)-0X1.0000000000001P-4)*1.0/(c_l*c_l*c_l*c_l)*m_l[12]+((double)0X0P+0)*1.0/(c_l*c_l)*m_l[13]+((double)0X0P+0)*1.0/(c_l*c_l)*m_l[14]+((double)-0X1P-2)*1.0/(c_l*c_l)*m_l[15]+((double)0X1P-3)*1.0/(c_l*c_l*c_l)*m_l[16]+((double)0X0P+0)*1.0/(c_l*c_l*c_l)*m_l[17]+((double)0X1P-3)*1.0/(c_l*c_l*c_l)*m_l[18];

m_inv_l[15]=+((double)0X1.C71C71C71C71CP-6)*1.0*m_l[0]+((double)0X1.5555555555555P-5)*1.0/(c_l*c_l)*m_l[1]+((double)0X1.C71C71C71C71CP-7)*1.0/(c_l*c_l*c_l*c_l)*m_l[2]+((double)0X0P+0)*1.0/c_l*m_l[3]+((double)0X0P+0)*1.0/(c_l*c_l*c_l)*m_l[4]+((double)0X1.5555555555555P-4)*1.0/(c_l)*m_l[5]+((double)0X1.5555555555555P-5)*1.0/(c_l*c_l*c_l)*m_l[6]+((double)0X1.5555555555555P-4)*1.0/c_l*m_l[7]+((double)0X1.5555555555555P-5)*1.0/(c_l*c_l*c_l)*m_l[8]+((double)-0X1.5555555555554P-5)*1.0/(c_l*c_l)*m_l[9]+((double)-0X1.5555555555554P-5)*1.0/(c_l*c_l*c_l*c_l)*m_l[10]+((double)0X0P+0)*1.0/(c_l*c_l)*m_l[11]+((double)0X0P+0)*1.0/(c_l*c_l*c_l*c_l)*m_l[12]+((double)0X0P+0)*1.0/(c_l*c_l)*m_l[13]+((double)0X1P-2)*1.0/(c_l*c_l)*m_l[14]+((double)0X0P+0)*1.0/(c_l*c_l)*m_l[15]+((double)0X0P+0)*1.0/(c_l*c_l*c_l)*m_l[16]+((double)0X1P-3)*1.0/(c_l*c_l*c_l)*m_l[17]+((double)-0X1P-3)*1.0/(c_l*c_l*c_l)*m_l[18];

m_inv_l[16]=+((double)0X1.C71C71C71C71AP-6)*1.0*m_l[0]+((double)0X1.5555555555557P-5)*1.0/(c_l*c_l)*m_l[1]+((double)0X1.C71C71C71C71AP-7)*1.0/(c_l*c_l*c_l*c_l)*m_l[2]+((double)0X0P+0)*1.0/c_l*m_l[3]+((double)0X0P+0)*1.0/(c_l*c_l*c_l)*m_l[4]+((double)-0X1.5555555555555P-4)*1.0/(c_l)*m_l[5]+((double)-0X1.5555555555555P-5)*1.0/(c_l*c_l*c_l)*m_l[6]+((double)-0X1.5555555555555P-4)*1.0/c_l*m_l[7]+((double)-0X1.5555555555555P-5)*1.0/(c_l*c_l*c_l)*m_l[8]+((double)-0X1.5555555555554P-5)*1.0/(c_l*c_l)*m_l[9]+((double)-0X1.5555555555554P-5)*1.0/(c_l*c_l*c_l*c_l)*m_l[10]+((double)0X0P+0)*1.0/(c_l*c_l)*m_l[11]+((double)0X0P+0)*1.0/(c_l*c_l*c_l*c_l)*m_l[12]+((double)0X0P+0)*1.0/(c_l*c_l)*m_l[13]+((double)0X1P-2)*1.0/(c_l*c_l)*m_l[14]+((double)0X0P+0)*1.0/(c_l*c_l)*m_l[15]+((double)0X0P+0)*1.0/(c_l*c_l*c_l)*m_l[16]+((double)-0X1P-3)*1.0/(c_l*c_l*c_l)*m_l[17]+((double)0X1P-3)*1.0/(c_l*c_l*c_l)*m_l[18];

m_inv_l[17]=+((double)0X1.C71C71C71C71CP-6)*1.0*m_l[0]+((double)0X1.5555555555555P-5)*1.0/(c_l*c_l)*m_l[1]+((double)0X1.C71C71C71C71CP-7)*1.0/(c_l*c_l*c_l*c_l)*m_l[2]+((double)0X0P+0)*1.0/c_l*m_l[3]+((double)0X0P+0)*1.0/(c_l*c_l*c_l)*m_l[4]+((double)0X1.5555555555555P-4)*1.0/(c_l)*m_l[5]+((double)0X1.5555555555555P-5)*1.0/(c_l*c_l*c_l)*m_l[6]+((double)-0X1.5555555555555P-4)*1.0/c_l*m_l[7]+((double)-0X1.5555555555555P-5)*1.0/(c_l*c_l*c_l)*m_l[8]+((double)-0X1.5555555555556P-5)*1.0/(c_l*c_l)*m_l[9]+((double)-0X1.5555555555556P-5)*1.0/(c_l*c_l*c_l*c_l)*m_l[10]+((double)0X0P+0)*1.0/(c_l*c_l)*m_l[11]+((double)0X0P+0)*1.0/(c_l*c_l*c_l*c_l)*m_l[12]+((double)0X0P+0)*1.0/(c_l*c_l)*m_l[13]+((double)-0X1P-2)*1.0/(c_l*c_l)*m_l[14]+((double)0X0P+0)*1.0/(c_l*c_l)*m_l[15]+((double)0X0P+0)*1.0/(c_l*c_l*c_l)*m_l[16]+((double)0X1P-3)*1.0/(c_l*c_l*c_l)*m_l[17]+((double)0X1P-3)*1.0/(c_l*c_l*c_l)*m_l[18];

m_inv_l[18]=+((double)0X1.C71C71C71C71CP-6)*1.0*m_l[0]+((double)0X1.5555555555555P-5)*1.0/(c_l*c_l)*m_l[1]+((double)0X1.C71C71C71C71CP-7)*1.0/(c_l*c_l*c_l*c_l)*m_l[2]+((double)-0X0P+0)*1.0/c_l*m_l[3]+((double)-0X0P+0)*1.0/(c_l*c_l*c_l)*m_l[4]+((double)-0X1.5555555555555P-4)*1.0/(c_l)*m_l[5]+((double)-0X1.5555555555555P-5)*1.0/(c_l*c_l*c_l)*m_l[6]+((double)0X1.5555555555555P-4)*1.0/c_l*m_l[7]+((double)0X1.5555555555555P-5)*1.0/(c_l*c_l*c_l)*m_l[8]+((double)-0X1.5555555555556P-5)*1.0/(c_l*c_l)*m_l[9]+((double)-0X1.5555555555556P-5)*1.0/(c_l*c_l*c_l*c_l)*m_l[10]+((double)-0X0P+0)*1.0/(c_l*c_l)*m_l[11]+((double)-0X0P+0)*1.0/(c_l*c_l*c_l*c_l)*m_l[12]+((double)-0X0P+0)*1.0/(c_l*c_l)*m_l[13]+((double)-0X1P-2)*1.0/(c_l*c_l)*m_l[14]+((double)0X0P+0)*1.0/(c_l*c_l)*m_l[15]+((double)-0X0P+0)*1.0/(c_l*c_l*c_l)*m_l[16]+((double)-0X1P-3)*1.0/(c_l*c_l*c_l)*m_l[17]+((double)-0X1P-3)*1.0/(c_l*c_l*c_l)*m_l[18];

//====================



			//==============================================================================
				
		
		//==========================TEST==NEW PERTUBATION===============================
		//===========WARRING: ONLY BE USED IF LATTICE VELOCITY=1 DT=DX=1===================
		//if (cc>0)
		//for (int mi=0;mi<19;mi++)
		//	{sum=9.0*CapA/4.0*s_v/2*cc*(w[mi]*(e[mi][0]*C[0]+e[mi][1]*C[1]+e[mi][2]*C[2])*(e[mi][0]*C[0]+e[mi][1]*C[1]+e[mi][2]*C[2])/(cc*cc)-cb[mi]);
		//	m_inv_l[mi]+=sum;
		//	//cout<<sum<<"    "<<endl;
		//	}

	
		//==============================================================================


		for (int mi=0; mi<19; mi++)
			{
			        
			        //if (m_inv_l[mi]!=m_inv_l[mi])
				//	cout<<n<<"	"<<mi<<"	"<<rank<<endl;




			if (nei_loc[ci][mi]>0)
			        F[nei_loc[ci][mi]][mi]=m_inv_l[mi];
			else
	                                   if (nei_loc[ci][mi]==0)
	                                            F[ci][LR[mi]]=m_inv_l[mi];
	                                    else
	                                    {
	                                            bufsend[-nei_loc[ci][mi]-1][sumtmp2[-nei_loc[ci][mi]-1]]=m_inv_l[mi];
	                                            sumtmp2[-nei_loc[ci][mi]-1]++;
	                                    }
			        
	
                 eu=elat[mi][0]*u[ci][0]+elat[mi][1]*u[ci][1]+elat[mi][2]*u[ci][2];
                 
		 g_r[mi]=w[mi]*rho_r[ci]*(1+3*eu/c2+4.5*eu*eu/c4-1.5*uu/c2); 
		
                 g_b[mi]=w[mi]*rho_b[ci]*(1+3*eu/c2+4.5*eu*eu/c4-1.5*uu/c2);
		
                 }

	   //cout<<"@@     "<<rank<<endl;         
                 
           if (cc>0)
           for(int kk=1;kk<19;kk+=2)
                {
                //ef=elat[kk][0]*C[0]+elat[kk][1]*C[1]+elat[kk][2]*C[2];
                ef=e[kk][0]*C[0]+e[kk][1]*C[1]+e[kk][2]*C[2];
                cospsi=g_r[kk]<g_r[kk+1]?g_r[kk]:g_r[kk+1];
                cospsi=cospsi<g_b[kk]?cospsi:g_b[kk];
                cospsi=cospsi<g_b[kk+1]?cospsi:g_b[kk+1];
                cospsi*=ef/cc;
		
		//cout<<"@@@@@     "<<ef/cc<<endl;

                g_r[kk]+=cospsi;
                g_r[kk+1]-=cospsi;
                g_b[kk]-=cospsi;
                g_b[kk+1]+=cospsi;
		
                }      
			
                      
	               
	               
	           //cout<<"**     "<<rank<<"	"<<ci<<"	"<<Count+1<<endl;    
			   
		       for(int mi=0;mi<19;mi++)


			{       
				if (nei_loc[ci][mi]>0)
				        {
				                rhor[nei_loc[ci][mi]]+=g_r[mi];
						rhob[nei_loc[ci][mi]]+=g_b[mi];
						//rhor[ci]+=g_r[mi];rhob[ci]+=g_b[mi];
						
						
				        }
				        else
				        if (nei_loc[ci][mi]==0)
				                {
				                        rhor[ci]+=g_r[mi];rhob[ci]+=g_b[mi];
				                }
				        else
				                {
	//cout<<com_n<<"	"<<-nei_loc[ci][mi]-1<<"	"<<mi<<"	"<<rank<<endl;
				                       bufsendrhor[-nei_loc[ci][mi]-1][sumtmp3[-nei_loc[ci][mi]-1]]=g_r[mi];
				                       bufsendrhob[-nei_loc[ci][mi]-1][sumtmp3[-nei_loc[ci][mi]-1]]=g_b[mi];
				                       sumtmp3[-nei_loc[ci][mi]-1]++; 
				                }  
				        

				}


		//cout<<"@@     "<<rank<<"	"<<ci<<"	"<<Count+1<<endl;
		
			
}

//cout<<"@@     "<<rank<<endl;    

MPI_Barrier(MPI_COMM_WORLD);

                        
	for (int i=0;i<com_n;i++)
	{
	MPI_Isend(bufsend[i],bufinfo[com_ind[i]], MPI_DOUBLE, com_ind[i]-1, (procind-1)*procn+com_ind[i]-1+3*procn*procn+3, MPI_COMM_WORLD,&request2[6*i]);
	                       
	MPI_Irecv(bufrecv[i],bufinfo[com_ind[i]], MPI_DOUBLE, com_ind[i]-1, (com_ind[i]-1)*procn+procind-1+3*procn*procn+3, MPI_COMM_WORLD,&request2[6*i+1]);
	
	MPI_Isend(bufsendrhor[i],bufinfo[com_ind[i]], MPI_DOUBLE, com_ind[i]-1, (procind-1)*procn+com_ind[i]-1+procn*procn+1, MPI_COMM_WORLD,&request2[6*i+2]);
	                       
	MPI_Irecv(bufrecvrhor[i],bufinfo[com_ind[i]], MPI_DOUBLE, com_ind[i]-1, (com_ind[i]-1)*procn+procind-1+procn*procn+1, MPI_COMM_WORLD,&request2[6*i+3]);
	
	MPI_Isend(bufsendrhob[i],bufinfo[com_ind[i]], MPI_DOUBLE, com_ind[i]-1, (procind-1)*procn+com_ind[i]-1+2*procn*procn+2, MPI_COMM_WORLD,&request2[6*i+4]);
	                       
	MPI_Irecv(bufrecvrhob[i],bufinfo[com_ind[i]], MPI_DOUBLE, com_ind[i]-1, (com_ind[i]-1)*procn+procind-1+2*procn*procn+2, MPI_COMM_WORLD,&request2[6*i+5]);
	}
	
	               MPI_Waitall(6*com_n,request2, status2);
	               MPI_Testall(6*com_n,request2,&mpi_test2,status2);


			
			//for (int i=0;i<com_n;i++)
	                //               for (int j=0;j<bufinfo[com_ind[i]];j++)
			//		if (bufrecv[i][j]!=bufrecv[i][j])
			//			cout<<"@@@@@   "<<n<<"	"<<com_ind[i]<<endl;
		
	               
	                     for (int i=0;i<com_n;i++)
	                       {
	                               for (int j=0;j<bufinfo[com_ind[i]];j++)
	                                       {
	                                               testl1=(int)(buflocrecv[i][j]/19);
	                                               testl2=(int)(buflocrecv[i][j]%19);
	                                               F[testl1][testl2]=bufrecv[i][j];
	                                               rhor[testl1]+=bufrecvrhor[i][j];
	                                               rhob[testl1]+=bufrecvrhob[i][j];
						
	                                       }
	                       }
	               
	               
/*	               
for (int i=0;i<com_n;i++)
	{
	if (sumtmp[i]!=bufinfo[com_ind[i]])
		cout<<procind<<"		No."<<0<<endl;

if (sumtmp2[i]!=bufinfo[com_ind[i]])
		cout<<procind<<"		No."<<1<<endl;

if (sumtmp3[i]!=bufinfo[com_ind[i]])
		cout<<procind<<"		No."<<2<<endl;

	//cout<<sumtmp[i]<<"	"<<bufinfo[com_ind[i]]<<"	"<<procind<<endl;
	}	

*/               
	
	

}


void standard_bounceback_boundary(int it,double** f)
{

	double tmp;
			tmp = f[it][1];f[it][1] = f[it][2];f[it][2] = tmp;
			tmp = f[it][3];f[it][3] = f[it][4];f[it][4] = tmp;
                        tmp = f[it][5];f[it][5] = f[it][6];f[it][6] = tmp;
			tmp = f[it][7];f[it][7] = f[it][10];f[it][10] = tmp;
			tmp = f[it][8];f[it][8] = f[it][9];f[it][9] = tmp;
			tmp = f[it][11];f[it][11] = f[it][14];f[it][14] = tmp;
                        tmp = f[it][12];f[it][12] = f[it][13];f[it][13] = tmp;
			tmp = f[it][15];f[it][15] = f[it][18];f[it][18] = tmp;
			tmp = f[it][16];f[it][16] = f[it][17];f[it][17] = tmp;


			

}



void comput_macro_variables( double* rho,double** u,double** u0,double** f,double** F,double* rho_r, double* rho_b, double* rhor, double* rhob, double* psi,int* SupInv)
{
	
	int rank = MPI :: COMM_WORLD . Get_rank ();
	int mpi_size=MPI :: COMM_WORLD . Get_size ();
	srand((unsigned)time(0)+rank);
	double rand_double,psi_rand;

//=============PSI FILETER==================
double cpc=0.98;
double psitemp;
//==========================================



	for(int i=1;i<=Count;i++)	
                   
			{
				//if ((u[i][0]!=u[i][0]) and (u0[i][0]==u0[i][0]))
				//	{
				//	cout<<n<<"	"<<rank<<"	"<<f[2][3]<<"	"<<rho[i]<<endl;
				//	}
				u0[i][0]=u[i][0];
				u0[i][1]=u[i][1];
				u0[i][2]=u[i][2];
				rho[i]=0;
				u[i][0]=0;
				u[i][1]=0;
				u[i][2]=0;
	
				for(int k=0;k<19;k++)
					{
					
					f[i][k]=F[i][k];
					rho[i]+=f[i][k]; 	//if (rho[i]!=rho[i]) cout<<f[i][k]<<"	"<<n<<"	"<<rank<<"	"<<k<<endl;
					u[i][0]+=elat[k][0]*f[i][k];
					u[i][1]+=elat[k][1]*f[i][k];
					u[i][2]+=elat[k][2]*f[i][k];
					}
				
				rho_r[i]=rhor[i];
				rho_b[i]=rhob[i];
				rhor[i]=0;
				rhob[i]=0;
				u[i][0]=(u[i][0]+dt*gxs/2)/rho[i];
				u[i][1]=(u[i][1]+dt*gys/2)/rho[i];
				u[i][2]=(u[i][2]+dt*gzs/2)/rho[i];
				
				
				psi[i]=(rho_r[i]-rho_b[i])/(rho_r[i]+rho_b[i]);


				//psi[i]=Psi_local[i];
			
			/*	
			//==================PSI FILTER=============================
			psitemp=(psi[i]+1)/2;
			psitemp=psitemp>cpc/2?psitemp:cpc/2;
			psitemp=psitemp<1-cpc/2?psitemp:1-cpc/2;
			psitemp-=cpc/2;
			psitemp=psitemp*(1.0/(1-cpc))*2-1;
			//cout<<psi[i]<<"        "<<psitemp<<endl;
			psi[i]=psitemp;
			//=========================================================
			*/

		
			}
			
		
	if (in_psi_BC==1)
                {
                        
                        
                   if (psi_xn>0)
                           if (psi_xn==1)   
                           {	
                                   for (int ii=0;ii<bclxn;ii++)
                                           {
                                               rho_r[bclx[ii]]=(Psi_local[bclx[ii]]*1.0+1.0)/2;
                                               rho_b[bclx[ii]]=1.0-rho_r[bclx[ii]];
                                               psi[bclx[ii]]=Psi_local[bclx[ii]];    
                                           }
                           }
                           else
                                   if (psi_xn==2)
                                   {
                                           for (int ii=0;ii<bclxn;ii++)
                                           if (nei_loc[bclx[ii]][1]>0)
                                           //for (int ii=0;ii<bclxn;ii++)
                                           {
                                            psi[bclx[ii]]=psi[nei_loc[bclx[ii]][1]];
                                            rho_r[bclx[ii]]=(psi[bclx[ii]]*1.0+1.0)/2;
                                            rho_b[bclx[ii]]=1.0-rho_r[bclx[ii]];
                                           }
                                           else
                                                    for (int ii=0;ii<bclxn;ii++)
                                                    {
                                                            rho_r[bclx[ii]]=(Psi_local[bclx[ii]]*1.0+1.0)/2;
                                                            rho_b[bclx[ii]]=1.0-rho_r[bclx[ii]];
                                                            psi[bclx[ii]]=Psi_local[bclx[ii]];    
                                                    }
                                   }
                                   else
                                           for (int ii=0;ii<bclxn;ii++)
                                           {
                                                   rand_double=(double(rand()%10000))/10000;
                                                   if (rand_double<ini_Sat)
                                                           psi_rand=1;
                                                   else
                                                           psi_rand=-1;
			
                                                   rho_r[bclx[ii]]=(psi_rand*1.0+1.0)/2;
                                                   rho_b[bclx[ii]]=1.0-rho_r[bclx[ii]];
                                                   psi[bclx[ii]]=psi_rand;
                                   
                                   }
                                   
                                   
                        
               if (psi_xp>0)
                           if (psi_xp==1)   
                           {
					//cout<<"@@@@@@@@@@"<<endl;
                                   for (int ii=0;ii<bcrxn;ii++)
                                           {
                                               rho_r[bcrx[ii]]=(Psi_local[bcrx[ii]]*1.0+1.0)/2;
                                               rho_b[bcrx[ii]]=1.0-rho_r[bcrx[ii]];
                                               psi[bcrx[ii]]=Psi_local[bcrx[ii]];    
                                           }
                           }
                           else
                                   if (psi_xp==2)
                                   {
                                           for (int ii=0;ii<bcrxn;ii++)
                                           if (nei_loc[bcrx[ii]][1]>0)
                                           //for (int ii=0;ii<bcrxn;ii++)
                                           {
                                            psi[bcrx[ii]]=psi[nei_loc[bcrx[ii]][1]];
                                            rho_r[bcrx[ii]]=(psi[bcrx[ii]]*1.0+1.0)/2;
                                            rho_b[bcrx[ii]]=1.0-rho_r[bcrx[ii]];
                                           }
                                           else
                                                    for (int ii=0;ii<bcrxn;ii++)
                                                    {
                                                            rho_r[bcrx[ii]]=(Psi_local[bcrx[ii]]*1.0+1.0)/2;
                                                            rho_b[bcrx[ii]]=1.0-rho_r[bcrx[ii]];
                                                            psi[bcrx[ii]]=Psi_local[bcrx[ii]];    
                                                    }
                                   }
                                   else
                                           for (int ii=0;ii<bcrxn;ii++)
                                           {
                                                   rand_double=(double(rand()%10000))/10000;
                                                   if (rand_double<ini_Sat)
                                                           psi_rand=1;
                                                   else
                                                           psi_rand=-1;
			
                                                   rho_r[bcrx[ii]]=(psi_rand*1.0+1.0)/2;
                                                   rho_b[bcrx[ii]]=1.0-rho_r[bcrx[ii]];
                                                   psi[bcrx[ii]]=psi_rand;
                                   
                                   }      
                                   
                                   
                                   
                                   
                     if (psi_yn>0)
                           if (psi_yn==1)   
                           {
                                   for (int ii=0;ii<bclyn;ii++)
                                           {
                                               rho_r[bcly[ii]]=(Psi_local[bcly[ii]]*1.0+1.0)/2;
                                               rho_b[bcly[ii]]=1.0-rho_r[bcly[ii]];
                                               psi[bcly[ii]]=Psi_local[bcly[ii]];    
                                           }
                           }
                           else
                                   if (psi_yn==2)
                                   {
                                           for (int ii=0;ii<bclyn;ii++)
                                           if (nei_loc[bcly[ii]][1]>0)
                                           //for (int ii=0;ii<bclyn;ii++)
                                           {
                                            psi[bcly[ii]]=psi[nei_loc[bcly[ii]][1]];
                                            rho_r[bcly[ii]]=(psi[bcly[ii]]*1.0+1.0)/2;
                                            rho_b[bcly[ii]]=1.0-rho_r[bcly[ii]];
                                           }
                                           else
                                                    for (int ii=0;ii<bclyn;ii++)
                                                    {
                                                            rho_r[bcly[ii]]=(Psi_local[bcly[ii]]*1.0+1.0)/2;
                                                            rho_b[bcly[ii]]=1.0-rho_r[bcly[ii]];
                                                            psi[bcly[ii]]=Psi_local[bcly[ii]];    
                                                    }
                                   }
                                   else
                                           for (int ii=0;ii<bclyn;ii++)
                                           {
                                                   rand_double=(double(rand()%10000))/10000;
                                                   if (rand_double<ini_Sat)
                                                           psi_rand=1;
                                                   else
                                                           psi_rand=-1;
			
                                                   rho_r[bcly[ii]]=(psi_rand*1.0+1.0)/2;
                                                   rho_b[bcly[ii]]=1.0-rho_r[bcly[ii]];
                                                   psi[bcly[ii]]=psi_rand;
                                   
                                   }
                                   
                                   
                        
               if (psi_yp>0)
                           if (psi_yp==1)   
                           {
                                   for (int ii=0;ii<bcryn;ii++)
                                           {
                                               rho_r[bcry[ii]]=(Psi_local[bcry[ii]]*1.0+1.0)/2;
                                               rho_b[bcry[ii]]=1.0-rho_r[bcry[ii]];
                                               psi[bcry[ii]]=Psi_local[bcry[ii]];
                                               //cout<<Psi_local[bcry[ii]]<<endl;
                                           }
                           }
                           else
                                   if (psi_yp==2)
                                   {
                                           for (int ii=0;ii<bcryn;ii++)
                                           if (nei_loc[bcry[ii]][1]>0)
                                           //for (int ii=0;ii<bcryn;ii++)
                                           {
                                            psi[bcry[ii]]=psi[nei_loc[bcry[ii]][1]];
                                            rho_r[bcry[ii]]=(psi[bcry[ii]]*1.0+1.0)/2;
                                            rho_b[bcry[ii]]=1.0-rho_r[bcry[ii]];
                                           }
                                           else
                                                    for (int ii=0;ii<bcryn;ii++)
                                                    {
                                                            rho_r[bcry[ii]]=(Psi_local[bcry[ii]]*1.0+1.0)/2;
                                                            rho_b[bcry[ii]]=1.0-rho_r[bcry[ii]];
                                                            psi[bcry[ii]]=Psi_local[bcry[ii]];    
                                                    }
                                   }
                                   else
                                           for (int ii=0;ii<bcryn;ii++)
                                           {
                                                   rand_double=(double(rand()%10000))/10000;
                                                   if (rand_double<ini_Sat)
                                                           psi_rand=1;
                                                   else
                                                           psi_rand=-1;
			
                                                   rho_r[bcry[ii]]=(psi_rand*1.0+1.0)/2;
                                                   rho_b[bcry[ii]]=1.0-rho_r[bcry[ii]];
                                                   psi[bcry[ii]]=psi_rand;
                                   
                                   }      
                                                 
                                   
                    if (psi_zn>0)
                           if (psi_zn==1)   
                           {
                                   for (int ii=0;ii<bclzn;ii++)
                                           {
                                               rho_r[bclz[ii]]=(Psi_local[bclz[ii]]*1.0+1.0)/2;
                                               rho_b[bclz[ii]]=1.0-rho_r[bclz[ii]];
                                               psi[bclz[ii]]=Psi_local[bclz[ii]];    
                                           }
                           }
                           else
                                   if (psi_zn==2)
                                   {
                                           for (int ii=0;ii<bclzn;ii++)
                                           if (nei_loc[bclz[ii]][1]>0)
                                           //for (int ii=0;ii<bclzn;ii++)
                                           {
                                            psi[bclz[ii]]=psi[nei_loc[bclz[ii]][1]];
                                            rho_r[bclz[ii]]=(psi[bclz[ii]]*1.0+1.0)/2;
                                            rho_b[bclz[ii]]=1.0-rho_r[bclz[ii]];
                                           }
                                           else
                                                    for (int ii=0;ii<bclzn;ii++)
                                                    {
                                                            rho_r[bclz[ii]]=(Psi_local[bclz[ii]]*1.0+1.0)/2;
                                                            rho_b[bclz[ii]]=1.0-rho_r[bclz[ii]];
                                                            psi[bclz[ii]]=Psi_local[bclz[ii]];    
                                                    }
                                   }
                                   else
                                           for (int ii=0;ii<bclzn;ii++)
                                           {
                                                   rand_double=(double(rand()%10000))/10000;
                                                   if (rand_double<ini_Sat)
                                                           psi_rand=1;
                                                   else
                                                           psi_rand=-1;
			
                                                   rho_r[bclz[ii]]=(psi_rand*1.0+1.0)/2;
                                                   rho_b[bclz[ii]]=1.0-rho_r[bclz[ii]];
                                                   psi[bclz[ii]]=psi_rand;
                                   
                                   }
                                   
                                   
                        
               if (psi_zp>0)
                           if (psi_zp==1)   
                           {
                                   for (int ii=0;ii<bcrzn;ii++)
                                           {
                                               rho_r[bcrz[ii]]=(Psi_local[bcrz[ii]]*1.0+1.0)/2;
                                               rho_b[bcrz[ii]]=1.0-rho_r[bcrz[ii]];
                                               psi[bcrz[ii]]=Psi_local[bcrz[ii]];    
                                           }
                           }
                           else
                                   if (psi_zp==2)
                                   {
                                           for (int ii=0;ii<bcrzn;ii++)
                                           if (nei_loc[bcrz[ii]][1]>0)
                                           //for (int ii=0;ii<bcrzn;ii++)
                                           {
                                            psi[bcrz[ii]]=psi[nei_loc[bcrz[ii]][1]];
                                            rho_r[bcrz[ii]]=(psi[bcrz[ii]]*1.0+1.0)/2;
                                            rho_b[bcrz[ii]]=1.0-rho_r[bcrz[ii]];
                                           }
                                           else
                                                    for (int ii=0;ii<bcrzn;ii++)
                                                    {
                                                            rho_r[bcrz[ii]]=(Psi_local[bcrz[ii]]*1.0+1.0)/2;
                                                            rho_b[bcrz[ii]]=1.0-rho_r[bcrz[ii]];
                                                            psi[bcrz[ii]]=Psi_local[bcrz[ii]];    
                                                    }
                                   }
                                   else
                                           for (int ii=0;ii<bcrzn;ii++)
                                           {
                                                   rand_double=(double(rand()%10000))/10000;
                                                   if (rand_double<ini_Sat)
                                                           psi_rand=1;
                                                   else
                                                           psi_rand=-1;
			
                                                   rho_r[bcrz[ii]]=(psi_rand*1.0+1.0)/2;
                                                   rho_b[bcrz[ii]]=1.0-rho_r[bcrz[ii]];
                                                   psi[bcrz[ii]]=psi_rand;
                                   
                                   }                     
                                   
                                   
                                   
                                   
                                   
                                   
                                   
                                   
                                   
                        
             /*           
                   if ((psi_xn>0) and (rank==0))
			if (psi_xn==1)    
                           {
                           for(int j=0;j<=NY;j++)
                                   for (int k=0;k<=NZ;k++)
				   if (Solid[0][j][k]>0)
                                   {
                                   rho_r[Solid[0][j][k]]=(Psi_local[0][j][k]*1.0+1.0)/2;
                                   rho_b[Solid[0][j][k]]=1.0-rho_r[Solid[0][j][k]];
                                   psi[Solid[0][j][k]]=Psi_local[0][j][k];
				//cout<<Psi_local[0][j][k]<<"  "<<j<<"  "<<k<<endl;
                                   
                                   
                                   }
                           }
			else
			if (psi_xn==2)
			{
                           for(int j=0;j<=NY;j++)
                                   for (int k=0;k<=NZ;k++)
				   if (Solid[0][j][k]>0)
				   if (Solid[1][j][k]>0)
                                   {
                                   psi[Solid[0][j][k]]=psi[Solid[1][j][k]];
                                   rho_r[Solid[0][j][k]]=(psi[Solid[0][j][k]]*1.0+1.0)/2;
                                   rho_b[Solid[0][j][k]]=1.0-rho_r[Solid[0][j][k]];
                                   
                                   }
                                   else
                                           {
                                             rho_r[Solid[0][j][k]]=(Psi_local[0][j][k]*1.0+1.0)/2;
                                             rho_b[Solid[0][j][k]]=1.0-rho_r[Solid[0][j][k]];
                                             psi[Solid[0][j][k]]=Psi_local[0][j][k];              
                                                   
                                           }
                        }
			else
			if (psi_xn==3)
			{
			for(int j=0;j<=NY;j++)
                                   for (int k=0;k<=NZ;k++)
				   if (Solid[0][j][k]>0)
                                   {
				   rand_double=(double(rand()%10000))/10000;
					if (rand_double<ini_Sat)
			        		psi_rand=1;
					else
			        		psi_rand=-1;
			
                                   rho_r[Solid[0][j][k]]=(psi_rand*1.0+1.0)/2;
                                   rho_b[Solid[0][j][k]]=1.0-rho_r[Solid[0][j][k]];
                                   psi[Solid[0][j][k]]=psi_rand;
                                   
                                   }
			}




                        
                      if ((psi_xp>0) and (rank==mpi_size-1))    
                        if (psi_xp==1)   
			{
                           for(int j=0;j<=NY;j++)
                                   for (int k=0;k<=NZ;k++)
				   if (Solid[nx_l-1][j][k]>0)
                                   {
                                   rho_r[Solid[nx_l-1][j][k]]=(Psi_local[nx_l-1][j][k]*1.0+1.0)/2;
                                   rho_b[Solid[nx_l-1][j][k]]=1.0-rho_r[Solid[nx_l-1][j][k]];
                                   psi[Solid[nx_l-1][j][k]]=Psi_local[nx_l-1][j][k];
                                   
                                   }
                           }  
			else
			if (psi_xp==2)
			{
                           for(int j=0;j<=NY;j++)
                                   for (int k=0;k<=NZ;k++)
				   if (Solid[nx_l-1][j][k]>0)
				   if (Solid[nx_l-2][j][k]>0) 
                                   {
                              
                                   psi[Solid[nx_l-1][j][k]]=psi[Solid[nx_l-2][j][k]];
                                   rho_r[Solid[nx_l-1][j][k]]=(psi[Solid[nx_l-1][j][k]]*1.0+1.0)/2;
                                   rho_b[Solid[nx_l-1][j][k]]=1.0-rho_r[Solid[nx_l-1][j][k]];
                                   }
                                   else
                                           {
                                            rho_r[Solid[nx_l-1][j][k]]=(Psi_local[nx_l-1][j][k]*1.0+1.0)/2;
                                            rho_b[Solid[nx_l-1][j][k]]=1.0-rho_r[Solid[nx_l-1][j][k]];
                                            psi[Solid[nx_l-1][j][k]]=Psi_local[nx_l-1][j][k];       
                                                   
                                           }
                           } 
			else 
			if (psi_xp==3)	
                        {
						
			for(int j=0;j<=NY;j++)
                             for (int k=0;k<=NZ;k++)
				   if (Solid[nx_l-1][j][k]>0)
                                   {
				rand_double=(double(rand()%10000))/10000;
					if (rand_double<ini_Sat)
			        		psi_rand=1;
					else
			        		psi_rand=-1;	
                                   rho_r[Solid[nx_l-1][j][k]]=(psi_rand*1.0+1.0)/2;
                                   rho_b[Solid[nx_l-1][j][k]]=1.0-rho_r[Solid[nx_l-1][j][k]];
                                   psi[Solid[nx_l-1][j][k]]=psi_rand;
                                   
                                   }


			}
                        
                         if (psi_yn==1)    
                           {
                           for(int i=0;i<nx_l;i++)
                                   for (int k=0;k<=NZ;k++)
				   if (Solid[i][0][k]>0)
                                   {
                                   rho_r[Solid[i][0][k]]=(Psi_local[i][0][k]*1.0+1.0)/2;
                                   rho_b[Solid[i][0][k]]=1.0-rho_r[Solid[i][0][k]];
                                   psi[Solid[i][0][k]]=Psi_local[i][0][k];
                                   
                                   }
                           }
			else 
			 if (psi_yn==2)    
                           {
                           for(int i=0;i<nx_l;i++)
                                   for (int k=0;k<=NZ;k++)
				   if (Solid[i][0][k]>0)
				   if (Solid[i][1][k]>0)
                                   {
                                   psi[Solid[i][0][k]]=psi[Solid[i][1][k]];
                                   rho_r[Solid[i][0][k]]=(psi[Solid[i][0][k]]*1.0+1.0)/2;
                                   rho_b[Solid[i][0][k]]=1.0-rho_r[Solid[i][0][k]];
                                   }
                                   else
                                           {
                                             rho_r[Solid[i][0][k]]=(Psi_local[i][0][k]*1.0+1.0)/2;
                                             rho_b[Solid[i][0][k]]=1.0-rho_r[Solid[i][0][k]];
                                             psi[Solid[i][0][k]]=Psi_local[i][0][k];      
                                                   
                                           }
                           }
			else
			if (psi_yn==3)
			{
                           for(int i=0;i<nx_l;i++)
                                   for (int k=0;k<=NZ;k++)
				   if (Solid[i][0][k]>0)
                                   {

					rand_double=(double(rand()%10000))/10000;
					if (rand_double<ini_Sat)
			        		psi_rand=1;
					else
			        		psi_rand=-1;
                                   rho_r[Solid[i][0][k]]=(psi_rand*1.0+1.0)/2;
                                   rho_b[Solid[i][0][k]]=1.0-rho_r[Solid[i][0][k]];
                                   psi[Solid[i][0][k]]=psi_rand;
                                   
                                   }
                           }
 
                        
                        if (psi_yp==1)  
                           {
                           for(int i=0;i<nx_l;i++)
                                   for (int k=0;k<=NZ;k++)
					if (Solid[i][NY][k]>0)
                                   {
                                   rho_r[Solid[i][NY][k]]=(Psi_local[i][NY][k]*1.0+1.0)/2;
                                   rho_b[Solid[i][NY][k]]=1.0-rho_r[Solid[i][NY][k]];
                                   psi[Solid[i][NY][k]]=Psi_local[i][NY][k];
                                   
                                   }
                           }  
			else
			if (psi_yp==2)  
                           {
                           for(int i=0;i<nx_l;i++)
                                   for (int k=0;k<=NZ;k++)
					if (Solid[i][NY][k]>0)
					if (Solid[i][NY-1][k]>0)
                                   {
                                   
                                   psi[Solid[i][NY][k]]=psi[Solid[i][NY-1][k]];
                                   rho_r[Solid[i][NY][k]]=(psi[Solid[i][NY][k]]*1.0+1.0)/2;
                                   rho_b[Solid[i][NY][k]]=1.0-rho_r[Solid[i][NY][k]];
                                   }
                                   else
                                           {
                                             rho_r[Solid[i][NY][k]]=(Psi_local[i][NY][k]*1.0+1.0)/2;
                                             rho_b[Solid[i][NY][k]]=1.0-rho_r[Solid[i][NY][k]];
                                             psi[Solid[i][NY][k]]=Psi_local[i][NY][k];       
                                           }
                           } 
				else
				if (psi_yp==3)  
				{
				for(int i=0;i<nx_l;i++)
                                   for (int k=0;k<=NZ;k++)
					if (Solid[i][NY][k]>0)
                                   {

					rand_double=(double(rand()%10000))/10000;
					if (rand_double<ini_Sat)
			        		psi_rand=1;
					else
			        		psi_rand=-1;
                                   rho_r[Solid[i][NY][k]]=(psi_rand*1.0+1.0)/2;
                                   rho_b[Solid[i][NY][k]]=1.0-rho_r[Solid[i][NY][k]];
                                   psi[Solid[i][NY][k]]=psi_rand;
                                   
                                   }
				} 


                        
                        if (psi_zn==1)    
                           {
                           for(int i=0;i<nx_l;i++)
                                   for (int j=0;j<=NY;j++)
					if (Solid[i][j][0]>0)
                                   {
                                   rho_r[Solid[i][j][0]]=(Psi_local[i][j][0]*1.0+1.0)/2;
                                   rho_b[Solid[i][j][0]]=1.0-rho_r[Solid[i][j][0]];
                                   psi[Solid[i][j][0]]=Psi_local[i][j][0];
                                   
                                   }
                           } 
			else
				 if (psi_zn==2)    
                           {
                           for(int i=0;i<nx_l;i++)
                                   for (int j=0;j<=NY;j++)
					if (Solid[i][j][0]>0)
					if (Solid[i][j][1]>0)
                                   {
                                   
                                   psi[Solid[i][j][0]]=psi[Solid[i][j][1]];
                                   rho_r[Solid[i][j][0]]=(psi[Solid[i][j][0]]*1.0+1.0)/2;
                                   rho_b[Solid[i][j][0]]=1.0-rho_r[Solid[i][j][0]];
                                   }
                                   else
                                           {
                                            rho_r[Solid[i][j][0]]=(Psi_local[i][j][0]*1.0+1.0)/2;
                                            rho_b[Solid[i][j][0]]=1.0-rho_r[Solid[i][j][0]];
                                            psi[Solid[i][j][0]]=Psi_local[i][j][0];       
                                           }
                           }
			else 
			if (psi_zn==3)
				{
				for(int i=0;i<nx_l;i++)
                                   for (int j=0;j<=NY;j++)
					if (Solid[i][j][0]>0)
                                   {
					rand_double=(double(rand()%10000))/10000;
					if (rand_double<ini_Sat)
			        		psi_rand=1;
					else
			        		psi_rand=-1;
                                   rho_r[Solid[i][j][0]]=(psi_rand*1.0+1.0)/2;
                                   rho_b[Solid[i][j][0]]=1.0-rho_r[Solid[i][j][0]];
                                   psi[Solid[i][j][0]]=psi_rand;
                                   
                                   }
				}
                        


                    if (psi_zp==1)    
                           {
                           for(int i=0;i<nx_l;i++)
                                   for (int j=0;j<=NY;j++)
					if (Solid[i][j][NZ]>0)
                                   {
                                   rho_r[Solid[i][j][NZ]]=(Psi_local[i][j][NZ]*1.0+1.0)/2;
                                   rho_b[Solid[i][j][NZ]]=1.0-rho_r[Solid[i][j][NZ]];
                                   psi[Solid[i][j][NZ]]=Psi_local[i][j][NZ];
                                   
                                   }
                           } 
		else
			if (psi_zp==2)    
                           {
                           for(int i=0;i<nx_l;i++)
                                   for (int j=0;j<=NY;j++)
					if (Solid[i][j][NZ]>0)
					if (Solid[i][j][NZ-1]>0)
                                   {
                                   
                                   psi[Solid[i][j][NZ]]=psi[Solid[i][j][NZ-1]];
                                   rho_r[Solid[i][j][NZ]]=(psi[Solid[i][j][NZ]]*1.0+1.0)/2;
                                   rho_b[Solid[i][j][NZ]]=1.0-rho_r[Solid[i][j][NZ]];
                                   }
                                   else
                                           {
                                           rho_r[Solid[i][j][NZ]]=(Psi_local[i][j][NZ]*1.0+1.0)/2;
                                           rho_b[Solid[i][j][NZ]]=1.0-rho_r[Solid[i][j][NZ]];
                                           psi[Solid[i][j][NZ]]=Psi_local[i][j][NZ];        
                                           }
                           } 
			else
			if (psi_zp==3)  
			   {
                           for(int i=0;i<nx_l;i++)
                                   for (int j=0;j<=NY;j++)
					if (Solid[i][j][NZ]>0)
                                   {
					rand_double=(double(rand()%10000))/10000;
					if (rand_double<ini_Sat)
			        		psi_rand=1;
					else
			        		psi_rand=-1;
                                   rho_r[Solid[i][j][NZ]]=(psi_rand*1.0+1.0)/2;
                                   rho_b[Solid[i][j][NZ]]=1.0-rho_r[Solid[i][j][NZ]];
                                   psi[Solid[i][j][NZ]]=psi_rand;
                                   
                                   }
                           }   
                     */   
                        
                }
	
         //for (int ii=0;ii<bcryn;ii++)
            //                               {
                                               //rho_r[bcry[ii]]=(Psi_local[bcry[ii]]*1.0+1.0)/2;
                                               //rho_b[bcry[ii]]=1.0-rho_r[bcry[ii]];
                                               //psi[bcry[ii]]=Psi_local[bcry[ii]];
                                               //cout<<(int)(coor[bcry[ii]]%((NY+1)*(NZ+1))/(NZ+1))<<endl;
               //                                cout<<psi[bcry[ii]]<<endl;
                                               
                                               
                  //                         }             
			
	//MPI_Barrier(MPI_COMM_WORLD); 

	//if (n>2100)
	//cout<<f[2][3]<<"		"<<rank<<endl;
}




void boundary_velocity(int xp,double v_xp,int xn, double v_xn,int yp,double v_yp,int yn,double v_yn,int zp,double v_zp,int zn,double v_zn,double** f,double** F,double* rho,double** u,int*** Solid)

{

int Q=19;
//double ux0,uy0,uz0;

double u_xp[3]={v_xp,0,0};
double u_xn[3]={v_xn,0,0};
double u_yp[3]={0,v_yp,0};
double u_yn[3]={0,v_yn,0};
double u_zp[3]={0,0,v_zp};
double u_zn[3]={0,0,v_zn};


int rank = MPI :: COMM_WORLD . Get_rank ();
int mpi_size=MPI :: COMM_WORLD . Get_size ();


//Equilibrium boundary condition (Use equilibrium distribution to update the distributions of particles on the boundaries)
if ((Sub_BC==0) or (Sub_BC==1))
{

if ((yp-1)*(yn-1)==0)
	{
	if (yn==1)
	for (int i=0;i<bclyn;i++)
		for (int ks=0;ks<Q;ks++)
		F[bcly[i]][ks]=feq(ks,1.0,u_yn); 
	if (yp==1)
	for (int i=0;i<bcryn;i++)
		for (int ks=0;ks<Q;ks++)
		F[bcry[i]][ks]=feq(ks,1.0,u_yp); 
	}
 


if ((zp-1)*(zn-1)==0)	
	{
	if (zn==1)
	for (int i=0;i<bclzn;i++)
		for (int ks=0;ks<Q;ks++)
		F[bclz[i]][ks]=feq(ks,1.0,u_zn); 

	if (zp==1)
	for (int i=0;i<bcrzn;i++)
		for (int ks=0;ks<Q;ks++)
		F[bcrz[i]][ks]=feq(ks,1.0,u_zp); 
	}
 


/*	
for (int i=0;i<nx_l;i++)
	for(int j=0;j<=NY;j++)
		for (int ks=0;ks<Q;ks++)
		{
		if ((zp==1) && (Solid[i][j][NZ]>0))    
		        F[Solid[i][j][NZ]][ks]=feq(ks,1.0,u_zp); 
		if ((zn==1) && (Solid[i][j][0]>0))
		        F[Solid[i][j][0]][ks]=feq(ks,1.0,u_zn);
		}

*/

if ((xp-1)*(xn-1)==0)
	{
	if (xn==1)
	for (int i=0;i<bclxn;i++)
		for (int ks=0;ks<Q;ks++)
		F[bclx[i]][ks]=feq(ks,1.0,u_xn); 

	if (xp==1)
	for (int i=0;i<bcrxn;i++)
		for (int ks=0;ks<Q;ks++)
		F[bcrx[i]][ks]=feq(ks,1.0,u_xp); 
	}


/*
if ((xp==1) && (rank==mpi_size-1))
for (int j=0;j<=NY;j++)
	for (int k=0;k<=NZ;k++)
		for (int ks=0;ks<Q;ks++)
			F[Solid[nx_l-1][j][k]][ks]=feq(ks,1.0,u_xp);
			



if ((xn==1) && (rank==0))
for (int j=0;j<=NY;j++)
	for(int k=0;k<=NZ;k++)
		for (int ks=0;ks<Q;ks++)
			F[Solid[0][j][k]][ks]=feq(ks,1.0,u_xn);
			
	
*/

}




//Non-Equilibrium boundary condition the distribution of BC points is updated using static pressure: rho=1.0
if (Sub_BC==2)
{
if (yp==1)
for (int i=0;i<bcryn;i++)
	for (int ks=0;ks<Q;ks++)
	if (e[ks][1]<0)
	F[bcry[i]][ks]=feq(LR[ks],1.0,u_yp)-F[bcry[i]][LR[ks]]+feq(ks,1.0,u_yp);

if (yn==1)
for (int i=0;i<bclyn;i++)
	for (int ks=0;ks<Q;ks++)
	if (e[ks][1]>0)
	F[bcly[i]][ks]=feq(LR[ks],1.0,u_yn)-F[bcly[i]][LR[ks]]+feq(ks,1.0,u_yn);


if (xp==1)
for (int i=0;i<bcrzn;i++)
	for (int ks=0;ks<Q;ks++)
	if (e[ks][2]<0)
	F[bcrz[i]][ks]=feq(LR[ks],1.0,u_zp)-F[bcrz[i]][LR[ks]]+feq(ks,1.0,u_zp);

if (xn==1)
for (int i=0;i<bclzn;i++)
	for (int ks=0;ks<Q;ks++)
	if (e[ks][2]>0)
	F[bclz[i]][ks]=feq(LR[ks],1.0,u_zn)-F[bclz[i]][LR[ks]]+feq(ks,1.0,u_zn);

if (zp==1)
for (int i=0;i<bcrxn;i++)
	for (int ks=0;ks<Q;ks++)
	if (e[ks][0]<0)
	F[bcrx[i]][ks]=feq(LR[ks],1.0,u_xp)-F[bcrx[i]][LR[ks]]+feq(ks,1.0,u_xp);

if (zn==1)
for (int i=0;i<bclxn;i++)
	for (int ks=0;ks<Q;ks++)
	if (e[ks][0]>0)
	F[bclx[i]][ks]=feq(LR[ks],1.0,u_xn)-F[bclx[i]][LR[ks]]+feq(ks,1.0,u_xn);

//cout<<"~~~~~~~~~~~~~~~@@@@@@@@@@@@"<<endl;
	
}

//Non-Equilibrium boundary condition use neighbouring points' value to update the BC points
if (Sub_BC==3)
{
/*
const int e[19][3]=
{{0,0,0},{1,0,0,},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1},{1,1,0},{-1,1,0},{1,-1,0},{-1,-1,0},{0,1,1},
{0,-1,1},{0,1,-1},{0,-1,-1},{1,0,1},{-1,0,1},{1,0,-1},{-1,0,-1}};
*/

if (yp==1)
for (int i=0;i<bcryn;i++)
	for (int ks=0;ks<Q;ks++)
	if (nei_loc[bcry[i]][4]>0)
	F[bcry[i]][ks]=feq(ks,rho[nei_loc[bcry[i]][4]],u_yp);
if (yn==1)
for (int i=0;i<bclyn;i++)
	for (int ks=0;ks<Q;ks++)
	if (nei_loc[bcly[i]][3]>0)
	F[bcly[i]][ks]=feq(ks,rho[nei_loc[bcly[i]][3]],u_yn);
if (xp==1)
for (int i=0;i<bcrxn;i++)
	for (int ks=0;ks<Q;ks++)
	if (nei_loc[bcrx[i]][2]>0)
	F[bcrx[i]][ks]=feq(ks,rho[nei_loc[bcrx[i]][2]],u_xp);
if (xn==1)
for (int i=0;i<bclxn;i++)
	for (int ks=0;ks<Q;ks++)
	if (nei_loc[bclx[i]][1]>0)
	F[bclx[i]][ks]=feq(ks,rho[nei_loc[bclx[i]][1]],u_xn);
if (zp==1)
for (int i=0;i<bcrzn;i++)
	for (int ks=0;ks<Q;ks++)
	if (nei_loc[bcrz[i]][6]>0)
	F[bcrz[i]][ks]=feq(ks,rho[nei_loc[bcrz[i]][6]],u_zp);
if (zn==1)
for (int i=0;i<bclzn;i++)
	for (int ks=0;ks<Q;ks++)
	if (nei_loc[bclz[i]][5]>0)
	F[bclz[i]][ks]=feq(ks,rho[nei_loc[bclz[i]][5]],u_zn);


}


}



void boundary_pressure(int xp,double rho_xp,int xn, double rho_xn,int yp,double rho_yp,int yn,double rho_yn,int zp,double rho_zp,int zn,double rho_zn,double** f,double** F,double** u,double* rho,int*** Solid)


{

int Q=19;
double u_ls[3]={0,0,0};
int rank = MPI :: COMM_WORLD . Get_rank ();
int mpi_size=MPI :: COMM_WORLD . Get_size ();
double m_l[19];

//Equilibriun boundary condition. velocities of boundary particles are set as 0.0
if (Sub_BC==0)
{
if (yp==1)
for (int i=0;i<bcryn;i++)
	for (int ks=0;ks<Q;ks++)
		F[bcry[i]][ks]=feq(ks,rho_yp,u_ls);
if (yn==1)	
for (int i=0;i<bclyn;i++)
	for (int ks=0;ks<Q;ks++)
		F[bcly[i]][ks]=feq(ks,rho_yn,u_ls);
if (xp==1)
for (int i=0;i<bcrxn;i++)
	for (int ks=0;ks<Q;ks++)
		F[bcrx[i]][ks]=feq(ks,rho_xp,u_ls);
if (xn==1)	
for (int i=0;i<bclxn;i++)
	for (int ks=0;ks<Q;ks++)
		F[bclx[i]][ks]=feq(ks,rho_xn,u_ls);
if (zp==1)
for (int i=0;i<bcrzn;i++)
	for (int ks=0;ks<Q;ks++)
		F[bcrz[i]][ks]=feq(ks,rho_zp,u_ls);
if (zn==1)	
for (int i=0;i<bclzn;i++)
	for (int ks=0;ks<Q;ks++)
		F[bclz[i]][ks]=feq(ks,rho_zn,u_ls);
		
}
       


//Equilibruim boundary condition: Use the value of neighbouring points values to update the distributions of BC points
 if (Sub_BC==1)
{


if (yp==1)
for (int i=0;i<bcryn;i++)
	if (nei_loc[bcry[i]][4]>0)
	for (int ks=0;ks<Q;ks++)
	{
	u_ls[0]=u[nei_loc[bcry[i]][4]][0];
	u_ls[1]=u[nei_loc[bcry[i]][4]][1];
	u_ls[2]=u[nei_loc[bcry[i]][4]][2];
	F[bcry[i]][ks]=feq(ks,rho_yp,u_ls);
	}
	else
		for (int ks=0;ks<Q;ks++)
		{
		u_ls[0]=0.0;
		u_ls[1]=0.0;u_ls[2]=0.0;
		F[bcry[i]][ks]=feq(ks,rho_yp,u_ls);
		}			

if (yn==1)	
for (int i=0;i<bclyn;i++)
	if (nei_loc[bcly[i]][3]>0)
	for (int ks=0;ks<Q;ks++)
	{
	u_ls[0]=u[nei_loc[bcly[i]][3]][0];
	u_ls[1]=u[nei_loc[bcly[i]][3]][1];
	u_ls[2]=u[nei_loc[bcly[i]][3]][2];
	F[bcly[i]][ks]=feq(ks,rho_yn,u_ls);
	}
	else
		for (int ks=0;ks<Q;ks++)
		{
		u_ls[0]=0.0;
		u_ls[1]=0.0;u_ls[2]=0.0;
		F[bcry[i]][ks]=feq(ks,rho_yn,u_ls);
		}

if (xp==1)
for (int i=0;i<bcrxn;i++)
	if (nei_loc[bcrx[i]][2]>0)
	for (int ks=0;ks<Q;ks++)
	{
	u_ls[0]=u[nei_loc[bcrx[i]][2]][0];
	u_ls[1]=u[nei_loc[bcrx[i]][2]][1];
	u_ls[2]=u[nei_loc[bcrx[i]][2]][2];
	F[bcrx[i]][ks]=feq(ks,rho_xp,u_ls);
	}
	else
		for (int ks=0;ks<Q;ks++)
		{
		u_ls[0]=0.0;
		u_ls[1]=0.0;u_ls[2]=0.0;
		F[bcry[i]][ks]=feq(ks,rho_xp,u_ls);
		}

if (xn==1)
for (int i=0;i<bclxn;i++)
	if (nei_loc[bclx[i]][1]>0)
	for (int ks=0;ks<Q;ks++)
	{
	u_ls[0]=u[nei_loc[bclx[i]][1]][0];
	u_ls[1]=u[nei_loc[bclx[i]][1]][1];
	u_ls[2]=u[nei_loc[bclx[i]][1]][2];
	F[bclx[i]][ks]=feq(ks,rho_xn,u_ls);
	}
	else
		for (int ks=0;ks<Q;ks++)
		{
		u_ls[0]=0.0;
		u_ls[1]=0.0;u_ls[2]=0.0;
		F[bcry[i]][ks]=feq(ks,rho_xn,u_ls);
		}


if (zp==1)
for (int i=0;i<bcrzn;i++)
	if (nei_loc[bcrz[i]][6]>0)
	for (int ks=0;ks<Q;ks++)
	{
	u_ls[0]=u[nei_loc[bcrz[i]][6]][0];
	u_ls[1]=u[nei_loc[bcrz[i]][6]][1];
	u_ls[2]=u[nei_loc[bcrz[i]][6]][2];
	F[bcrz[i]][ks]=feq(ks,rho_zp,u_ls);
	}
	else
		for (int ks=0;ks<Q;ks++)
		{
		u_ls[0]=0.0;
		u_ls[1]=0.0;u_ls[2]=0.0;
		F[bcry[i]][ks]=feq(ks,rho_zp,u_ls);
		}


if (zn==1)
for (int i=0;i<bclzn;i++)
	if (nei_loc[bclz[i]][5]>0)
	for (int ks=0;ks<Q;ks++)
	{
	u_ls[0]=u[nei_loc[bclz[i]][5]][0];
	u_ls[1]=u[nei_loc[bclz[i]][5]][1];
	u_ls[2]=u[nei_loc[bclz[i]][5]][2];
	F[bclz[i]][ks]=feq(ks,rho_zn,u_ls);
	}
	else
		for (int ks=0;ks<Q;ks++)
		{
		u_ls[0]=0.0;
		u_ls[1]=0.0;u_ls[2]=0.0;
		F[bcry[i]][ks]=feq(ks,rho_zn,u_ls);
		}



}	  


//Non-equilibrium boundary condition  
if (Sub_BC==2)
{


if (yp==1)
for (int i=0;i<bcryn;i++)
	if (nei_loc[bcry[i]][4]>0)
	{
	for (int mi=0; mi<19; mi++)
		{
		m_l[mi]=0;
		for (int mj=0; mj<19; mj++)
			m_l[mi]+=M[mi][mj]*f[nei_loc[bcry[i]][4]][mj];
		}
		m_l[0]=rho_yp;
		for (int mi=0; mi<19; mi++)
			{
			F[bcry[i]][mi]=0;
			for (int mj=0; mj<19; mj++)
				F[bcry[i]][mi]+=MI[mi][mj]*m_l[mj];
			}
	}
	else
		for (int ks=0;ks<Q;ks++)
		{
		u_ls[0]=0.0;
		u_ls[1]=0.0;u_ls[2]=0.0;
		F[bcry[i]][ks]=feq(ks,rho_yp,u_ls);
		}	


if (yn==1)
for (int i=0;i<bclyn;i++)
	if (nei_loc[bcly[i]][3]>0)
	{
	for (int mi=0; mi<19; mi++)
		{
		m_l[mi]=0;
		for (int mj=0; mj<19; mj++)
			m_l[mi]+=M[mi][mj]*f[nei_loc[bcly[i]][3]][mj];
		}
		m_l[0]=rho_yn;
		for (int mi=0; mi<19; mi++)
			{
			F[bcly[i]][mi]=0;
			for (int mj=0; mj<19; mj++)
				F[bcly[i]][mi]+=MI[mi][mj]*m_l[mj];
			}
	}
	else
		for (int ks=0;ks<Q;ks++)
		{
		u_ls[0]=0.0;
		u_ls[1]=0.0;u_ls[2]=0.0;
		F[bcly[i]][ks]=feq(ks,rho_yn,u_ls);
		}



if (xp==1)
for (int i=0;i<bcrxn;i++)
	if (nei_loc[bcrx[i]][2]>0)
	{
	for (int mi=0; mi<19; mi++)
		{
		m_l[mi]=0;
		for (int mj=0; mj<19; mj++)
			m_l[mi]+=M[mi][mj]*f[nei_loc[bcrx[i]][2]][mj];
		}
		m_l[0]=rho_xp;
		for (int mi=0; mi<19; mi++)
			{
			F[bcrx[i]][mi]=0;
			for (int mj=0; mj<19; mj++)
				F[bcrx[i]][mi]+=MI[mi][mj]*m_l[mj];
			}
	}
	else
		for (int ks=0;ks<Q;ks++)
		{
		u_ls[0]=0.0;
		u_ls[1]=0.0;u_ls[2]=0.0;
		F[bcrx[i]][ks]=feq(ks,rho_xp,u_ls);
		}	


if (xn==1)
for (int i=0;i<bclxn;i++)
	if (nei_loc[bclx[i]][1]>0)
	{
	for (int mi=0; mi<19; mi++)
		{
		m_l[mi]=0;
		for (int mj=0; mj<19; mj++)
			m_l[mi]+=M[mi][mj]*f[nei_loc[bclx[i]][1]][mj];
		}
		m_l[0]=rho_xn;
		for (int mi=0; mi<19; mi++)
			{
			F[bclx[i]][mi]=0;
			for (int mj=0; mj<19; mj++)
				F[bclx[i]][mi]+=MI[mi][mj]*m_l[mj];
			}
	}
	else
		for (int ks=0;ks<Q;ks++)
		{
		u_ls[0]=0.0;
		u_ls[1]=0.0;u_ls[2]=0.0;
		F[bclx[i]][ks]=feq(ks,rho_xn,u_ls);
		}



if (zp==1)
for (int i=0;i<bcrzn;i++)
	if (nei_loc[bcrz[i]][6]>0)
	{
	for (int mi=0; mi<19; mi++)
		{
		m_l[mi]=0;
		for (int mj=0; mj<19; mj++)
			m_l[mi]+=M[mi][mj]*f[nei_loc[bcrz[i]][6]][mj];
		}
		m_l[0]=rho_zp;
		for (int mi=0; mi<19; mi++)
			{
			F[bcrz[i]][mi]=0;
			for (int mj=0; mj<19; mj++)
				F[bcrz[i]][mi]+=MI[mi][mj]*m_l[mj];
			}
	}
	else
		for (int ks=0;ks<Q;ks++)
		{
		u_ls[0]=0.0;
		u_ls[1]=0.0;u_ls[2]=0.0;
		F[bcrz[i]][ks]=feq(ks,rho_zp,u_ls);
		}	

if (zn==1)
for (int i=0;i<bclzn;i++)
	if (nei_loc[bclz[i]][5]>0)
	{
	for (int mi=0; mi<19; mi++)
		{
		m_l[mi]=0;
		for (int mj=0; mj<19; mj++)
			m_l[mi]+=M[mi][mj]*f[nei_loc[bclz[i]][5]][mj];
		}
		m_l[0]=rho_zn;
		for (int mi=0; mi<19; mi++)
			{
			F[bclz[i]][mi]=0;
			for (int mj=0; mj<19; mj++)
				F[bclz[i]][mi]+=MI[mi][mj]*m_l[mj];
			}
	}
	else
		for (int ks=0;ks<Q;ks++)
		{
		u_ls[0]=0.0;
		u_ls[1]=0.0;u_ls[2]=0.0;
		F[bclz[i]][ks]=feq(ks,rho_zn,u_ls);
		}




}		

//Non-equilibrium boundary condition
if (Sub_BC==3)
{



if (yp==1)
for (int i=0;i<bcryn;i++)
	if (nei_loc[bcry[i]][4]>0)
	for (int ks=0;ks<Q;ks++)
	{
	u_ls[0]=u[nei_loc[bcry[i]][4]][0];
	u_ls[1]=u[nei_loc[bcry[i]][4]][1];
	u_ls[2]=u[nei_loc[bcry[i]][4]][2];
	F[bcry[i]][ks]=feq(ks,rho_yp,u_ls)+f[nei_loc[bcry[i]][4]][ks]-feq(ks,rho[nei_loc[bcry[i]][4]],u[nei_loc[bcry[i]][4]]);
	}
	else
		for (int ks=0;ks<Q;ks++)
		{
		u_ls[0]=0.0;
		u_ls[1]=0.0;u_ls[2]=0.0;
		F[bcry[i]][ks]=feq(ks,rho_yp,u_ls);
		}


if (yn==1)
for (int i=0;i<bclyn;i++)
	if (nei_loc[bcly[i]][3]>0)
	for (int ks=0;ks<Q;ks++)
	{
	u_ls[0]=u[nei_loc[bcly[i]][3]][0];
	u_ls[1]=u[nei_loc[bcly[i]][3]][1];
	u_ls[2]=u[nei_loc[bcly[i]][3]][2];
	F[bcly[i]][ks]=feq(ks,rho_yn,u_ls)+f[nei_loc[bcly[i]][4]][ks]-feq(ks,rho[nei_loc[bcly[i]][3]],u[nei_loc[bcly[i]][3]]);
	}
	else
		for (int ks=0;ks<Q;ks++)
		{
		u_ls[0]=0.0;
		u_ls[1]=0.0;u_ls[2]=0.0;
		F[bcly[i]][ks]=feq(ks,rho_yn,u_ls);
		}

if (xp==1)
for (int i=0;i<bcrxn;i++)
	if (nei_loc[bcrx[i]][2]>0)
	for (int ks=0;ks<Q;ks++)
	{
	u_ls[0]=u[nei_loc[bcrx[i]][2]][0];
	u_ls[1]=u[nei_loc[bcrx[i]][2]][1];
	u_ls[2]=u[nei_loc[bcrx[i]][2]][2];
	F[bcrx[i]][ks]=feq(ks,rho_xp,u_ls)+f[nei_loc[bcrx[i]][4]][ks]-feq(ks,rho[nei_loc[bcrx[i]][2]],u[nei_loc[bcrx[i]][2]]);
	}
	else
		for (int ks=0;ks<Q;ks++)
		{
		u_ls[0]=0.0;
		u_ls[1]=0.0;u_ls[2]=0.0;
		F[bcrx[i]][ks]=feq(ks,rho_xp,u_ls);
		}


if (xn==1)
for (int i=0;i<bclxn;i++)
	if (nei_loc[bclx[i]][1]>0)
	for (int ks=0;ks<Q;ks++)
	{
	u_ls[0]=u[nei_loc[bclx[i]][1]][0];
	u_ls[1]=u[nei_loc[bclx[i]][1]][1];
	u_ls[2]=u[nei_loc[bclx[i]][1]][2];
	F[bclx[i]][ks]=feq(ks,rho_xn,u_ls)+f[nei_loc[bclx[i]][4]][ks]-feq(ks,rho[nei_loc[bclx[i]][1]],u[nei_loc[bclx[i]][1]]);
	}
	else
		for (int ks=0;ks<Q;ks++)
		{
		u_ls[0]=0.0;
		u_ls[1]=0.0;u_ls[2]=0.0;
		F[bclx[i]][ks]=feq(ks,rho_xn,u_ls);
		}



if (zp==1)
for (int i=0;i<bcrzn;i++)
	if (nei_loc[bcrz[i]][6]>0)
	for (int ks=0;ks<Q;ks++)
	{
	u_ls[0]=u[nei_loc[bcrz[i]][6]][0];
	u_ls[1]=u[nei_loc[bcrz[i]][6]][1];
	u_ls[2]=u[nei_loc[bcrz[i]][6]][2];
	F[bcrz[i]][ks]=feq(ks,rho_zp,u_ls)+f[nei_loc[bcrz[i]][4]][ks]-feq(ks,rho[nei_loc[bcrz[i]][6]],u[nei_loc[bcrz[i]][6]]);
	}
	else
		for (int ks=0;ks<Q;ks++)
		{
		u_ls[0]=0.0;
		u_ls[1]=0.0;u_ls[2]=0.0;
		F[bcrz[i]][ks]=feq(ks,rho_zp,u_ls);
		}


if (zn==1)
for (int i=0;i<bclzn;i++)
	if (nei_loc[bclz[i]][5]>0)
	for (int ks=0;ks<Q;ks++)
	{
	u_ls[0]=u[nei_loc[bclz[i]][5]][0];
	u_ls[1]=u[nei_loc[bclz[i]][5]][1];
	u_ls[2]=u[nei_loc[bclz[i]][5]][2];
	F[bclz[i]][ks]=feq(ks,rho_zn,u_ls)+f[nei_loc[bclz[i]][4]][ks]-feq(ks,rho[nei_loc[bclz[i]][5]],u[nei_loc[bclz[i]][5]]);
	}
	else
		for (int ks=0;ks<Q;ks++)
		{
		u_ls[0]=0.0;
		u_ls[1]=0.0;u_ls[2]=0.0;
		F[bclz[i]][ks]=feq(ks,rho_zn,u_ls);
		}





}	
}


double Error(double** u,double** u0,double *v_max,double* u_average)
{	
	int rank = MPI :: COMM_WORLD . Get_rank ();
	int mpi_size=MPI :: COMM_WORLD . Get_size ();
	double *rbuf,*um,*uave,u_compt;
	double temp1,temp2,temp3;
	temp1=0;
	temp2=0;
	temp3=0;
	double error_in;
	double u_max;
	

	rbuf=new double[mpi_size];
	um = new double[mpi_size];
	uave = new double[mpi_size];
	u_max=0;
	*v_max=0;




//MPI_Barrier(MPI_COMM_WORLD);



for(int i=1; i<=Count; i++)
			{	

			temp1+=(u[i][0]-u0[i][0])*(u[i][0]-u0[i][0])+(u[i][1]-u0[i][1])*(u[i][1]-u0[i][1])+(u[i][2]-u0[i][2])*(u[i][2]-u0[i][2]);
			temp2 += u[i][0]*u[i][0]+u[i][1]*u[i][1]+u[i][2]*u[i][2];	
			temp3+=sqrt(u[i][0]*u[i][0]+u[i][1]*u[i][1]+u[i][2]*u[i][2]);
			if (u[i][0]*u[i][0]+u[i][1]*u[i][1]+u[i][2]*u[i][2]>u_max)
				u_max=u[i][0]*u[i][0]+u[i][1]*u[i][1]+u[i][2]*u[i][2];
			
			}
		
		temp1=sqrt(temp1);
		temp2=sqrt(temp2);
		error_in=temp1/(temp2+1e-30);
		u_max=sqrt(u_max);

		MPI_Barrier(MPI_COMM_WORLD);
		
	MPI_Gather(&error_in,1,MPI_DOUBLE,rbuf,1,MPI_DOUBLE,0,MPI_COMM_WORLD);
		
	MPI_Gather(&u_max,1,MPI_DOUBLE,um,1,MPI_DOUBLE,0,MPI_COMM_WORLD);

	MPI_Gather(&temp3,1,MPI_DOUBLE,uave,1,MPI_DOUBLE,0,MPI_COMM_WORLD);

	u_compt=0;
	if (rank==0)
	    for (int i=0;i<mpi_size;i++)
		{
		if (rbuf[i]>error_in)
			error_in=rbuf[i];
		if (um[i]>*v_max)
			*v_max=um[i];
		u_compt+=uave[i];

		}

	u_compt/=(NX+1)*(NY+1)*(NZ+1);
	*u_average=u_compt;
	
	delete [] rbuf;
	delete [] um;
	delete [] uave;

	//MPI_Barrier(MPI_COMM_WORLD);

	return(error_in);

}


void Geometry(int*** Solid)	
{	
	
	int rank = MPI :: COMM_WORLD . Get_rank ();
	ostringstream name;
	name<<outputfile<<"LBM_Geometry"<<".vtk";
	if (rank==0)
	{

	ofstream out;
	out.open(name.str().c_str());
	out<<"# vtk DataFile Version 2.0"<<endl;
	out<<"J.Yang Lattice Boltzmann Simulation 3D Single Phase-Solid-Geometry"<<endl;
	out<<"ASCII"<<endl;
	out<<"DATASET STRUCTURED_POINTS"<<endl;
	out<<"DIMENSIONS         "<<NX+1<<"         "<<NY+1<<"         "<<NZ+1<<endl;
	out<<"ORIGIN 0 0 0"<<endl;
	out<<"SPACING 1 1 1"<<endl;
	out<<"POINT_DATA     "<<(NX+1)*(NY+1)*(NZ+1)<<endl;
	out<<"SCALARS sample_scalars float"<<endl;
	out<<"LOOKUP_TABLE default"<<endl;

	

	for(int k=0;k<NZ+1;k++)
		for(int j=0; j<NY+1; j++)
			for(int i=0;i<NX+1;i++)
			if (Solid[i][j][k]>0)
				out<<0<<" ";
			else
				out<<1<<" ";


	out.close();
	}
	MPI_Barrier(MPI_COMM_WORLD);
	
	
	
	
		
}


void Geometry_Par(int*** Solid)	
{	
	int rank = MPI :: COMM_WORLD . Get_rank ();
	ostringstream name;
	name<<outputfile<<"LBM_Geometry_Mesh"<<".vtk";
	if (rank==0)
	{

	ofstream out;
	out.open(name.str().c_str());
	out<<"# vtk DataFile Version 2.0"<<endl;
	out<<"J.Yang Lattice Boltzmann Simulation 3D Single Phase-Solid-Geometry"<<endl;
	out<<"ASCII"<<endl;
	out<<"DATASET STRUCTURED_POINTS"<<endl;
	out<<"DIMENSIONS         "<<NX+1<<"         "<<NY+1<<"         "<<NZ+1<<endl;
	out<<"ORIGIN 0 0 0"<<endl;
	out<<"SPACING 1 1 1"<<endl;
	out<<"POINT_DATA     "<<(NX+1)*(NY+1)*(NZ+1)<<endl;
	out<<"SCALARS sample_scalars float"<<endl;
	out<<"LOOKUP_TABLE default"<<endl;

	

	for(int k=0;k<NZ+1;k++)
		for(int j=0; j<NY+1; j++)
			for(int i=0;i<NX+1;i++)
			out<<Solid[i][j][k]<<" ";


	out.close();
	}
	MPI_Barrier(MPI_COMM_WORLD);
	
	
}


void output_velocity(int m,double* rho,double** u,int MirX,int MirY,int MirZ,int mir,int*** Solid)	
{
	
	int rank = MPI :: COMM_WORLD . Get_rank ();
	const int mpi_size=MPI :: COMM_WORLD . Get_size ();
	int procind=rank+1;
	int procn=mpi_size;

	int tmpsum[mpi_size];
	for (int i=0;i<mpi_size;i++)
		tmpsum[i]=3;	

	const int root_rank=0;
	
	double rho_0=1.0;
	
	
	MPI_Status status;
	MPI_Request request;

	double* rbuf_v;



	int nx_g[mpi_size];
	int disp[mpi_size];
	
	for (int i=0;i<mpi_size;i++)
		nx_g[i]=(sumss[i+1]+1)*3;


	

		disp[0]=0;
	
		for (int i=1;i<mpi_size;i++)
			disp[i]=disp[i-1]+nx_g[i-1];
		
	
	if (rank==root_rank)
		rbuf_v = new double[disp[mpi_size-1]+nx_g[mpi_size-1]];

	
	int NX0=NX+1;
	int NY0=NY+1;
	int NZ0=NZ+1;
	MPI_Gatherv(u[0],nx_g[rank],MPI_DOUBLE,rbuf_v,nx_g,disp,MPI_DOUBLE,root_rank,MPI_COMM_WORLD);



	ostringstream name;
	name<<outputfile<<"LBM_velocity_Vector_"<<m<<".vtk";
	if (rank==root_rank)
	{

	ofstream out;
	out.open(name.str().c_str());
	out<<"# vtk DataFile Version 2.0"<<endl;
	out<<"J.Yang Lattice Boltzmann Simulation 3D Single Phase-Velocity"<<endl;
	out<<"ASCII"<<endl;
	out<<"DATASET STRUCTURED_POINTS"<<endl;
	out<<"DIMENSIONS         "<<NX0<<"         "<<NY0<<"         "<<NZ0<<endl;
	out<<"ORIGIN 0 0 0"<<endl;
	out<<"SPACING 1 1 1"<<endl;

	out<<"POINT_DATA     "<<NX0*NY0*NZ0<<endl;
	out<<"VECTORS sample_vectors double"<<endl;
	out<<endl;

	for(int k=0 ; k<NZ0 ; k++)			
	         for(int j=0 ; j<NY0 ; j++)
	                for(int i=0 ; i<NX0 ; i++)
			if (Solid[i][j][k]==0)
				out<<0<<" "<<0<<" "<<0<<endl;
			else
				{
				out<<rbuf_v[disp[Solid[i][j][k]-1]+tmpsum[Solid[i][j][k]-1]]<<" ";
				out<<rbuf_v[disp[Solid[i][j][k]-1]+tmpsum[Solid[i][j][k]-1]+1]<<" ";
				out<<rbuf_v[disp[Solid[i][j][k]-1]+tmpsum[Solid[i][j][k]-1]+2]<<endl;
				tmpsum[Solid[i][j][k]-1]+=3;
				}
	
	out.close();

	}
        MPI_Barrier(MPI_COMM_WORLD);
	
	if (rank==root_rank)
	delete [] rbuf_v;
		
}


void output_density(int m,double* rho,int MirX,int MirY,int MirZ,int mir,int*** Solid)	
{

	int rank = MPI :: COMM_WORLD . Get_rank ();
	const int mpi_size=MPI :: COMM_WORLD . Get_size ();
	int procind=rank+1;
	int procn=mpi_size;

	int tmpsum[mpi_size];
	for (int i=0;i<mpi_size;i++)
		tmpsum[i]=1;	

	const int root_rank=0;
	
	double rho_0=1.0;
	
	
	MPI_Status status;
	MPI_Request request;

	double* rbuf_v;



	int nx_g[mpi_size];
	int disp[mpi_size];
	
	for (int i=0;i<mpi_size;i++)
		nx_g[i]=sumss[i+1]+1;


	

		disp[0]=0;
	
		for (int i=1;i<mpi_size;i++)
			disp[i]=disp[i-1]+nx_g[i-1];
		
	
	if (rank==root_rank)
		rbuf_v = new double[disp[mpi_size-1]+nx_g[mpi_size-1]];

	
	int NX0=NX+1;
	int NY0=NY+1;
	int NZ0=NZ+1;
	MPI_Gatherv(rho,nx_g[rank],MPI_DOUBLE,rbuf_v,nx_g,disp,MPI_DOUBLE,root_rank,MPI_COMM_WORLD);




	ostringstream name;
	name<<outputfile<<"LBM_Density_"<<m<<".vtk";
	if (rank==root_rank)
	{
	ofstream out;
	out.open(name.str().c_str());
	out<<"# vtk DataFile Version 2.0"<<endl;
	out<<"J.Yang Lattice Boltzmann Simulation 3D Single Phase-Solid-Density"<<endl;
	out<<"ASCII"<<endl;
	out<<"DATASET STRUCTURED_POINTS"<<endl;
	out<<"DIMENSIONS         "<<NX0<<"         "<<NY0<<"         "<<NZ0<<endl;
	out<<"ORIGIN 0 0 0"<<endl;
	out<<"SPACING 1 1 1"<<endl;
	out<<"POINT_DATA     "<<NX0*NY0*NZ0<<endl;
	out<<"SCALARS sample_scalars float"<<endl;
	out<<"LOOKUP_TABLE default"<<endl;

	for(int k=0 ; k<NZ0 ; k++)			
	         for(int j=0 ; j<NY0 ; j++)
	                for(int i=0 ; i<NX0 ; i++)
			if (Solid[i][j][k]==0)
				out<<rho_0<<" ";
			else
				{
				out<<rbuf_v[disp[Solid[i][j][k]-1]+tmpsum[Solid[i][j][k]-1]]<<" ";
				tmpsum[Solid[i][j][k]-1]++;
				}
	
	out.close();

	}
        MPI_Barrier(MPI_COMM_WORLD);
	
	if (rank==root_rank)
	delete [] rbuf_v;

}

void output_psi(int m,double* psi,int MirX,int MirY,int MirZ,int mir,int*** Solid)	
{

	int rank = MPI :: COMM_WORLD . Get_rank ();
	const int mpi_size=MPI :: COMM_WORLD . Get_size ();
	int procind=rank+1;
	int procn=mpi_size;

	int tmpsum[mpi_size];
	for (int i=0;i<mpi_size;i++)
		tmpsum[i]=1;	

	const int root_rank=0;
	
	double rho_0=0.0;
	
	
	MPI_Status status;
	MPI_Request request;

	double* rbuf_v;



	int nx_g[mpi_size];
	int disp[mpi_size];
	
	for (int i=0;i<mpi_size;i++)
		nx_g[i]=sumss[i+1]+1;


	

		disp[0]=0;
	
		for (int i=1;i<mpi_size;i++)
			disp[i]=disp[i-1]+nx_g[i-1];
		
	
	if (rank==root_rank)
		rbuf_v = new double[disp[mpi_size-1]+nx_g[mpi_size-1]];

	
	int NX0=NX+1;
	int NY0=NY+1;
	int NZ0=NZ+1;
	MPI_Gatherv(psi,nx_g[rank],MPI_DOUBLE,rbuf_v,nx_g,disp,MPI_DOUBLE,root_rank,MPI_COMM_WORLD);




	ostringstream name;
	name<<outputfile<<"LBM_psi_"<<m<<".vtk";
	if (rank==root_rank)
	{
	ofstream out;
	out.open(name.str().c_str());
	out<<"# vtk DataFile Version 2.0 - "<<niu_l<<" "<<niu_g<<" "<<ContactAngle_parameter<<" "<<CapA<<endl;
	out<<"J.Yang Lattice Boltzmann Simulation 3D MC- "<<S_l<<" "<<porosity<<" "<<reso<<" "<<gxs<<" "<<p_xp<<" "<<p_xn<<endl;
	out<<"ASCII"<<endl;
	out<<"DATASET STRUCTURED_POINTS"<<endl;
	out<<"DIMENSIONS         "<<NX0<<"         "<<NY0<<"         "<<NZ0<<endl;
	out<<"ORIGIN 0 0 0"<<endl;
	out<<"SPACING 1 1 1"<<endl;
	out<<"POINT_DATA     "<<NX0*NY0*NZ0<<endl;
	out<<"SCALARS sample_scalars float"<<endl;
	out<<"LOOKUP_TABLE default"<<endl;

	for(int k=0 ; k<NZ0 ; k++)			
	         for(int j=0 ; j<NY0 ; j++)
	                for(int i=0 ; i<NX0 ; i++)
			if (Solid[i][j][k]==0)
				out<<rho_0<<" ";
			else
				{
				out<<setprecision(preci)<<rbuf_v[disp[Solid[i][j][k]-1]+tmpsum[Solid[i][j][k]-1]]<<" ";
				tmpsum[Solid[i][j][k]-1]++;
				}
	
	out.close();

	}
        MPI_Barrier(MPI_COMM_WORLD);
	
	if (rank==root_rank)
	delete [] rbuf_v;

}






double Comput_Perm(double* psi,double** u,double* Per_l,double* Per_g,int PerDIr, int* SupInv)
{


	int rank = MPI :: COMM_WORLD . Get_rank ();
	int mpi_size=MPI :: COMM_WORLD . Get_size ();

	
	int si,sj,sm;
	
	

	double *rbuf_l, *rbuf_g;
	rbuf_l=new double[mpi_size*3];
	rbuf_g=new double[mpi_size*3];
	
	
	double Perm_l[3];
	double Perm_g[3];
	double error,vxl,vyl,vzl;
	double Q_l[3]={0.0,0.0,0.0};
	double Q_g[3]={0.0,0.0,0.0};
	
	double dp;
	if (in_BC==0)
	        dp=0;
	else
	switch(PerDIr)
		{
		case 1:
			dp=abs(p_xp-p_xn)*c_s2/(NX+1)/dx;break;
		case 2:
			dp=abs(p_yp-p_yn)*c_s2/(NY+1)/dx;break;
		case 3:
			dp=abs(p_zp-p_zn)*c_s2/(NZ+1)/dx;break;
		default:
			dp=abs(p_xp-p_xn)*c_s2/(NX+1)/dx;
		}
		
		
	if ((par_per_x-1)*(par_per_y-1)*(par_per_z-1)==0)
	{	
	for (int i=1;i<=Count;i++)
		if (sqrt(u[i][0]*u[i][0]+u[i][1]*u[i][1]+u[i][2]*u[i][2])<rel_perm_psi2)
		{
		si=(int)(coor[i]/((NY+1)*(NZ+1)));
		sj=(int)((coor[i]%((NY+1)*(NZ+1)))/(NZ+1));
		sm=(int)(coor[i]%(NZ+1)); 
		
		

		//if (rank==1)
		//cout<<rank<<"  "<<si<<" "<<sj<<" "<<sm<<endl;
		//cout<<si<<"  "<<per_xn<<"  "<<per_xp<<endl;
			if ((si>=per_xn) and (si<=per_xp) and (sj>=per_yn) and (sj<=per_yp) and (sm>=per_zn) and (sm<=per_zp))
			{
	        		if (psi[i]>=rel_perm_psi)
		        	{
		                Q_l[0]+=u[i][0];
		                Q_l[1]+=u[i][1];
		                Q_l[2]+=u[i][2];
		        	}
				if (psi[i]<=-rel_perm_psi)
		        	{
                                Q_g[0]+=u[i][0];
                                Q_g[1]+=u[i][1];
                                Q_g[2]+=u[i][2];
		        
		        
		        	}
			}


		}
	}
	else
	for (int i=1;i<=Count;i++)
		if (sqrt(u[i][0]*u[i][0]+u[i][1]*u[i][1]+u[i][2]*u[i][2])<rel_perm_psi2)
		{
	        if (psi[i]>=rel_perm_psi)
		        {
		                Q_l[0]+=u[i][0];
		                Q_l[1]+=u[i][1];
		                Q_l[2]+=u[i][2];
		        }
		if (psi[i]<=-rel_perm_psi)
		        {
                                Q_g[0]+=u[i][0];
                                Q_g[1]+=u[i][1];
                                Q_g[2]+=u[i][2];
		        
		        
		        }
		}







	MPI_Barrier(MPI_COMM_WORLD);

	

	MPI_Gather(&Q_l,3,MPI_DOUBLE,rbuf_l,3,MPI_DOUBLE,0,MPI_COMM_WORLD);
	MPI_Gather(&Q_g,3,MPI_DOUBLE,rbuf_g,3,MPI_DOUBLE,0,MPI_COMM_WORLD);
	
	
	if (rank==0)
		{
		Q_l[0]=0;Q_l[1]=0;Q_l[2]=0;
		Q_g[0]=0;Q_g[1]=0;Q_g[2]=0;
		for (int i=0;i<mpi_size;i++)
			{
			Q_g[0]+=rbuf_g[i*3+0];
			Q_g[1]+=rbuf_g[i*3+1];
			Q_g[2]+=rbuf_g[i*3+2];
			Q_l[0]+=rbuf_l[i*3+0];
			Q_l[1]+=rbuf_l[i*3+1];
			Q_l[2]+=rbuf_l[i*3+2];
			}

		Perm_l[0]=Q_l[0]/((per_xp-per_xn+1)*(per_yp-per_yn+1)*(per_zp-per_zn+1))*(niu_l)/(gxs+dp);
		Perm_l[1]=Q_l[1]/((per_xp-per_xn+1)*(per_yp-per_yn+1)*(per_zp-per_zn+1))*(niu_l)/(gys+dp);
		Perm_l[2]=Q_l[2]/((per_xp-per_xn+1)*(per_yp-per_yn+1)*(per_zp-per_zn+1))*(niu_l)/(gzs+dp);

		Perm_g[0]=Q_g[0]/((per_xp-per_xn+1)*(per_yp-per_yn+1)*(per_zp-per_zn+1))*(niu_g)/(gxs+dp);
		Perm_g[1]=Q_g[1]/((per_xp-per_xn+1)*(per_yp-per_yn+1)*(per_zp-per_zn+1))*(niu_g)/(gys+dp);
		Perm_g[2]=Q_g[2]/((per_xp-per_xn+1)*(per_yp-per_yn+1)*(per_zp-per_zn+1))*(niu_g)/(gzs+dp);


		vxl=(Q_l[0]+Q_g[0])/((per_xp-per_xn+1)*(per_yp-per_yn+1)*(per_zp-per_zn+1));
		vyl=(Q_l[1]+Q_g[1])/((per_xp-per_xn+1)*(per_yp-per_yn+1)*(per_zp-per_zn+1));
		vzl=(Q_l[2]+Q_g[2])/((per_xp-per_xn+1)*(per_yp-per_yn+1)*(per_zp-per_zn+1));
		Capillary=sqrt(vxl*vxl+vyl*vyl+vzl*vzl)/porosity*niu_l/CapA;   

		u_ave=sqrt((vxl/porosity)*(vxl/porosity)+(vyl/porosity)*(vyl/porosity)+(vzl/porosity)*(vzl/porosity));

		switch(PerDIr)
		{
		case 1:
			error=(Perm_l[0]-Per_l[0])/Per_l[0];break;
		case 2:
			error=(Perm_l[1]-Per_l[1])/Per_l[1];break;
		case 3:
			error=(Perm_l[2]-Per_l[2])/Per_l[2];break;
		default:
			error=(Perm_l[0]-Per_l[0])/Per_l[0];
		}


		Per_l[0]=Perm_l[0];Per_g[0]=Perm_g[0];
		Per_l[1]=Perm_l[1];Per_g[1]=Perm_g[1];
		Per_l[2]=Perm_l[2];Per_g[2]=Perm_g[2];

		}
	
	delete [] rbuf_l;
	delete [] rbuf_g;
	
	
	return (error);
	

}




double Comput_Saturation(double* psi,int*** Solid,int* SupInv)
{
	int rank = MPI :: COMM_WORLD . Get_rank ();
	int mpi_size=MPI :: COMM_WORLD . Get_size ();

	double S_l,S_g;
	

	int si,sj,sm,sdir;
	

	
double *rbuf_l,*rbuf_g;

	rbuf_l=new double[mpi_size];
	rbuf_g=new double[mpi_size];

	S_l=0;S_g=0;
	Sd_l=0;Sd_g=0;
	
	
if ((par_per_x-1)*(par_per_y-1)*(par_per_z-1)==0)	
	for (int i=1;i<=Count;i++)
	{
		si=(int)(coor[i]/((NY+1)*(NZ+1)));
		sj=(int)((coor[i]%((NY+1)*(NZ+1)))/(NZ+1));
		sm=(int)(coor[i]%(NZ+1)); 
	
	
		
		//cout<<per_xn<<"        "<<per_xp<<"        "<<per_yn<<"        "<<per_yp<<"        "<<per_zn<<"        "<<per_zp<<endl;
		//cout<<si<<"        "<<sj<<"        "<<sm<<endl;
		
		if ((si>=per_xn) and (si<=per_xp) and (sj>=per_yn) and (sj<=per_yp) and (sm>=per_zn) and (sm<=per_zp))
		{
	
			if (psi[i]>=0) 
			S_l+=1;
			else
			S_g+=1;
		
		
		//============saturation displacement================= 
		        if (PerDir==1)
			sdir=si;
			else
			if (PerDir==2)
				sdir=sj;
			else
				sdir=sm;
				
			if (psi[i]>=0) 
			Sd_l+=psi[i]*sdir;
			else
			Sd_g+=psi[i]*sdir;
		//===========================================
		
		}
		
		
	}
	else
	for (int i=1;i<=Count;i++)
			{				
			if (psi[i]>=0) 
			S_l+=1;
			else
			S_g+=1;
		//cout<<"@@@@@@@@@"<<endl;
		
		//===========saturation displacement==================
		si=(int)(coor[i]/((NY+1)*(NZ+1)));
		sj=(int)((coor[i]%((NY+1)*(NZ+1)))/(NZ+1));
		sm=(int)(coor[i]%(NZ+1)); 
		

		if (PerDir==1)
			sdir=si;
			else
			if (PerDir==2)
				sdir=sj;
			else
				sdir=sm;
				
			if (psi[i]>=0) 
			Sd_l+=psi[i]*sdir;
			else
			Sd_g+=psi[i]*sdir;
		//===========================================
		        
		
			}

	
		
	

		MPI_Gather(&S_l,1,MPI_DOUBLE,rbuf_l,1,MPI_DOUBLE,0,MPI_COMM_WORLD);
	
		MPI_Gather(&S_g,1,MPI_DOUBLE,rbuf_g,1,MPI_DOUBLE,0,MPI_COMM_WORLD);
	
	if (rank==0)
	{S_l=0;S_g=0;
	for (int i=0;i<mpi_size;i++)
			{
			S_l+=rbuf_l[i];S_g+=rbuf_g[i];
			}
	
	//cout<<S_l<<"        ##########        "<<endl;
	//cout<<porosity<<endl;
	S_l=S_l/((per_xp-per_xn+1)*(per_yp-per_yn+1)*(per_zp-per_zn+1)*porosity);
	S_g=S_g/((per_xp-per_xn+1)*(per_yp-per_yn+1)*(per_zp-per_zn+1)*porosity);
	}

	MPI_Bcast(&S_l,1,MPI_DOUBLE,0,MPI_COMM_WORLD);
	
	//==================saturation displacement==============================
	        MPI_Gather(&Sd_l,1,MPI_DOUBLE,rbuf_l,1,MPI_DOUBLE,0,MPI_COMM_WORLD);
	        MPI_Gather(&Sd_g,1,MPI_DOUBLE,rbuf_g,1,MPI_DOUBLE,0,MPI_COMM_WORLD);
	
	if (rank==0)
	{
	Sd_l=0;Sd_g=0;
	for (int i=0;i<mpi_size;i++)
			{
			Sd_l+=rbuf_l[i];Sd_g+=rbuf_g[i];
			
			}	
	}

	
	
	MPI_Bcast(&Sd_l,1,MPI_DOUBLE,0,MPI_COMM_WORLD);
	MPI_Bcast(&Sd_g,1,MPI_DOUBLE,0,MPI_COMM_WORLD);
        //==================================================================	
	
	//cout<<S_l<<"      @@@@@@          "<<S_g<<endl;
	
	
	delete [] rbuf_l;
	delete [] rbuf_g;
	
	return (S_l);
			

}






void Backup_init(double* rho, double** u, double** f,double* psi,double* rho_r, double* rho_b, double* rhor, double* rhob,int* SupInv)
{	
      
        int rank = MPI :: COMM_WORLD . Get_rank ();
	int mpi_size=MPI :: COMM_WORLD . Get_size ();


	int nx_g[mpi_size];
	int disp[mpi_size];
	
	
	MPI_Gather(&nx_l,1,MPI_INT,nx_g,1,MPI_INT,0,MPI_COMM_WORLD);
	
	
	if (rank==0)
		{
		disp[0]=0;
	
		for (int i=1;i<mpi_size;i++)
			disp[i]=disp[i-1]+nx_g[i-1];
		
		}

	MPI_Bcast(disp,mpi_size,MPI_INT,0,MPI_COMM_WORLD);


	double usqr,vsqr,eu;
	double c2,c4;
	
	rho0=1.0;dt=1.0/Zoom;dx=1.0/Zoom;
 	uMax=0.0;

	if (lattice_v==1)
		{dx=dx_input;dt=dt_input;}

	lat_c=dx/dt;
	c_s=lat_c/sqrt(3);
	c_s2=lat_c*lat_c/3;

	c2=lat_c*lat_c;c4=c2*c2;

	niu=in_vis;
	tau_f=niu/(c_s2*dt)+0.5;
	//tau_f=3.0*niu/dt+0.5;
	int loc_x,loc_y,loc_z;
	
	s_v=1/tau_f;
       
	double s_other=8*(2-s_v)/(8-s_v);
	double u_tmp[3];
	
	
	
//=====VISCOSITY INTERPOLATION======================
	delta=0.1;
	wl=1.0/(niu_l/(c_s2*dt)+0.5);
	wg=1.0/(niu_g/(c_s2*dt)+0.5);
	lg0=2*wl*wg/(wl+wg);
	l1=2*(wl-lg0)/delta;
	l2=-l1/(2*delta);
	g1=2*(lg0-wg)/delta;
	g2=g1/(2*delta);
//=================================================
	
	
	
	
	S[0]=0;
	S[1]=s_v;
	S[2]=s_v;
	S[3]=0;
	S[4]=s_other;
	S[5]=0;
	S[6]=s_other;
	S[7]=0;
	S[8]=s_other;
	S[9]=s_v;
	S[10]=s_v;
	S[11]=s_v;
	S[12]=s_v;
	S[13]=s_v;
	S[14]=s_v;
	S[15]=s_v;
	S[16]=s_other;
	S[17]=s_other;
	S[18]=s_other;
	
	if (lattice_v==1)
	{
	
	M_c[0]=1.0;
	M_c[1]=lat_c*lat_c;
	M_c[2]=lat_c*lat_c*lat_c*lat_c;
	M_c[3]=lat_c;
	M_c[4]=lat_c*lat_c*lat_c;
	M_c[5]=lat_c;
	M_c[6]=lat_c*lat_c*lat_c;
	M_c[7]=lat_c;	
	M_c[8]=lat_c*lat_c*lat_c;
	M_c[9]=lat_c*lat_c;
	M_c[10]=lat_c*lat_c*lat_c*lat_c;
	M_c[11]=lat_c*lat_c;
	M_c[12]=lat_c*lat_c*lat_c*lat_c;
	M_c[13]=lat_c*lat_c;
	M_c[14]=lat_c*lat_c;
	M_c[15]=lat_c*lat_c;
	M_c[16]=lat_c*lat_c*lat_c;
	M_c[17]=lat_c*lat_c*lat_c;
	M_c[18]=lat_c*lat_c*lat_c;



	for (int i=0;i<19;i++)
		for (int j=0;j<3;j++)
		elat[i][j]=e[i][j]*lat_c;

	for (int i=0;i<19;i++)
		for (int j=0;j<19;j++)
		M[i][j]*=M_c[i];

	Comput_MI(M,MI);

	}


	psi_solid=ContactAngle_parameter;
	
	ostringstream name5;
	name5<<pfix<<"LBM_checkpoint_rhor_"<<mode_backup_ini<<"."<<rank<<".bin_input";
 	ostringstream name4;
	name4<<pfix<<"LBM_checkpoint_f_"<<mode_backup_ini<<"."<<rank<<".bin_input";
	ostringstream name3;
	name3<<pfix<<"LBM_checkpoint_psi_"<<mode_backup_ini<<"."<<rank<<".bin_input";
 	ostringstream name2;
	name2<<pfix<<"LBM_checkpoint_velocity_"<<mode_backup_ini<<"."<<rank<<".bin_input";
	ostringstream name;
	name<<pfix<<"LBM_checkpoint_rho_"<<mode_backup_ini<<"."<<rank<<".bin_input";
	ostringstream name6;
	name6<<pfix<<"LBM_checkpoint_rhob_"<<mode_backup_ini<<"."<<rank<<".bin_input";
	 
	fstream fin;
	fin.open(name.str().c_str(),ios::in);
	if (fin.fail())
	        {
	        cout<<"\n file open error on" << name.str().c_str()<<endl;
	        exit(-1);
	        }
	fin.read((char *)(&rho[0]), sizeof(double)*(Count+1));
 
       fin.close();
       
   
	fin.open(name2.str().c_str(),ios::in);
	if (fin.fail())
	        {
	        cout<<"\n file open error on" << name2.str().c_str()<<endl;
	        exit(-1);
	        }
	fin.read((char *)(&u[0][0]), sizeof(double)*(Count+1)*3);
      
       fin.close();
       
       fin.open(name3.str().c_str(),ios::in);
       if (fin.fail())
	        {
	        cout<<"\n file open error on" << name3.str().c_str()<<endl;
	        exit(-1);
	        }
       fin.read((char *)(&psi[0]), sizeof(double)*(Count+1));
     
       fin.close();
       
       fin.open(name4.str().c_str(),ios::in);
       if (fin.fail())
	        {
	        cout<<"\n file open error on" << name4.str().c_str()<<endl;
	        exit(-1);
	        }
	fin.read((char *)(&f[0][0]), sizeof(double)*(Count+1)*19);
      
       fin.close();
       
       fin.open(name5.str().c_str(),ios::in);
       if (fin.fail())
	        {
	        cout<<"\n file open error on" << name5.str().c_str()<<endl;
	        exit(-1);
	        }
	fin.read((char *)(&rho_r[0]), sizeof(double)*(Count+1));
        
       fin.close();

       fin.open(name6.str().c_str(),ios::in);
       if (fin.fail())
	        {
	        cout<<"\n file open error on" << name6.str().c_str()<<endl;
	        exit(-1);
	        }
	fin.read((char *)(&rho_b[0]), sizeof(double)*(Count+1));
        
       fin.close();
       
       //cout<<psi[3]<<endl;
       
       
        	for(int i=1;i<=Count;i++)
        	{
        	       
			rhor[i]=0;
			rhob[i]=0;
		}
  


/*
  for (int i=1;i<=Count;i++)	
			
		{     
      if ((par_per_x>0) or (par_per_y>0) or (par_per_z>0))
			        {        
			                loc_x=(int)(SupInv[i]/((NY+1)*(NZ+1)))+disp[rank];
			                loc_y=(int)((SupInv[i]%((NY+1)*(NZ+1)))/(NZ+1));
			                loc_z=SupInv[i]%(NZ+1);
			                if (((loc_x<per_xn) or (loc_x>per_xp) or (loc_y<per_yn) or (loc_y>per_yp) or (loc_z<per_zn) or (loc_z>per_zp)) and ((ini_buf==-1) or (ini_buf==1)))
			                {        
			                        psi[i]=ini_buf;
			                        rho_r[i]=(psi[i]*rho[i]+rho[i])/2;
			                        rho_b[i]=rho[i]-rho_r[i];

						//===============WARRNING==TEST==CODE========
						Psi_local[(int)(SupInv[i]/((NY+1)*(NZ+1)))][loc_y][loc_z]=ini_buf;
						//===========================================
			                       
			                }
			                
			                
			        }
	
		}

*/

	
		       if (stab==1)
				{gxs=0;gys=0;gzs=0;}
			else
				{gxs=gx;gys=gy;gzs=gz;}
			
	
	 	
}




void Backup(int m,double* rho,double* psi, double** u, double** f, double* rho_r, double* rho_b)
{

        int rank = MPI :: COMM_WORLD . Get_rank ();
	int mpi_size=MPI :: COMM_WORLD . Get_size ();
	
	ostringstream name;
	name<<outputfile<<"LBM_checkpoint_velocity_"<<m<<"."<<rank<<".bin_input";
	ofstream out;
	out.open(name.str().c_str());
	
        out.write((char *)(&u[0][0]),sizeof(double)*(Count+1)*3);	
			
	out.close();
	

	
	
	ostringstream name2;
	name2<<outputfile<<"LBM_checkpoint_rho_"<<m<<"."<<rank<<".bin_input";
	ofstream out2;
	out2.open(name2.str().c_str());
	

	out2.write((char *)(&rho[0]), sizeof(double)*(Count+1));	
			
	out2.close();
	

	
	ostringstream name3;
	name3<<outputfile<<"LBM_checkpoint_psi_"<<m<<"."<<rank<<".bin_input";
	//ofstream out;
	out.open(name3.str().c_str());
	
	
	out.write((char *)(&psi[0]), sizeof(double)*(Count+1));	
	
	out.close();
	
	
	ostringstream name4;
	name4<<outputfile<<"LBM_checkpoint_f_"<<m<<"."<<rank<<".bin_input";
	//ofstream out;
	out.open(name4.str().c_str());

        out.write((char *)(&f[0][0]), sizeof(double)*(Count+1)*19);      
        out.close();
        
	
        ostringstream name5;
	name5<<outputfile<<"LBM_checkpoint_rhor_"<<m<<"."<<rank<<".bin_input";
	//ofstream out2;
	out2.open(name5.str().c_str());
	

	out2.write((char *)(&rho_r[0]), sizeof(double)*(Count+1));	
			
	out2.close();

	ostringstream name6;
	name6<<outputfile<<"LBM_checkpoint_rhob_"<<m<<"."<<rank<<".bin_input";
	//ofstream out2;
	out2.open(name6.str().c_str());
	

	out2.write((char *)(&rho_b[0]), sizeof(double)*(Count+1));	
			
	out2.close();

}

void input_update()
{
	int rank = MPI :: COMM_WORLD . Get_rank ();
	int mpi_size=MPI :: COMM_WORLD . Get_size ();

	


if (rank==0)
	{
	ifstream fin(inp_up);
	                                                fin.getline(dummy, NCHAR);
	/*fin >> filename;*/				fin.getline(dummy, NCHAR);
	/*fin >> filenamepsi;*/				fin.getline(dummy, NCHAR);
	/*fin >> NX >> NY >> NZ;*/				fin.getline(dummy, NCHAR);
	fin >> n_max;					fin.getline(dummy, NCHAR);
	/*fin >> reso;*/					fin.getline(dummy, NCHAR);
	/*fin >> in_BC;*/					fin.getline(dummy, NCHAR);
	/*fin >> in_psi_BC;*/				fin.getline(dummy, NCHAR);
	fin >> gx >> gy >> gz;				fin.getline(dummy, NCHAR);
	/*fin >> pre_xp >> p_xp >> pre_xn >> p_xn;*/	fin.getline(dummy, NCHAR);
	/*fin >> pre_yp >> p_yp >> pre_yn >> p_yn;*/	fin.getline(dummy, NCHAR);
	/*fin >> pre_zp >> p_zp >> pre_zn >> p_zn;*/	fin.getline(dummy, NCHAR);
	/*fin >> vel_xp >> v_xp >> vel_xn >> v_xn;*/	fin.getline(dummy, NCHAR);
	/*fin >> vel_yp >> v_yp >> vel_yn >> v_yn;*/	fin.getline(dummy, NCHAR);
	/*fin >> vel_zp >> v_zp >> vel_zn >> v_zn;*/	fin.getline(dummy, NCHAR);
	/*fin >> psi_xp >> psi_xn;*/			fin.getline(dummy, NCHAR);
	/*fin >> psi_yp >> psi_yn;*/			fin.getline(dummy, NCHAR);
	/*fin >> psi_zp >> psi_zn;*/			fin.getline(dummy, NCHAR);
	/*fin >> niu_l;*/					fin.getline(dummy, NCHAR);
	/*fin >> niu_g;*/					fin.getline(dummy, NCHAR);
	fin >> ContactAngle_parameter;			fin.getline(dummy, NCHAR);
	fin >> CapA;					fin.getline(dummy, NCHAR);
	/*fin >> inivx >> inivy >> inivz;*/		fin.getline(dummy, NCHAR);
	fin >> Permeability;				fin.getline(dummy, NCHAR);
							fin.getline(dummy, NCHAR);
	/*fin >> wr_per;*/					fin.getline(dummy, NCHAR);
	/*fin >> PerDir;*/					fin.getline(dummy, NCHAR);
	fin >> freRe;					fin.getline(dummy, NCHAR);
	/*fin >> Out_Mode;*/				fin.getline(dummy, NCHAR);
	fin >> freVe;					fin.getline(dummy, NCHAR);
	fin >> freDe;					fin.getline(dummy, NCHAR);
	fin >> frePsi;					fin.getline(dummy, NCHAR);
							fin.getline(dummy, NCHAR);
	/*fin >> lattice_v >> dx_input >> dt_input;*/	fin.getline(dummy, NCHAR);
	/*fin >> outputfile;*/				fin.getline(dummy, NCHAR);
	/*fin >> Sub_BC;*/					fin.getline(dummy, NCHAR);
	/*fin >> stab >> stab_time;*/		fin.getline(dummy, NCHAR);
	/*fin >> ini_Sat;*/                                 fin.getline(dummy, NCHAR);
	/*fin >> ini_buf;*/                                 fin.getline(dummy, NCHAR);
	fin >> rel_perm_psi>>rel_perm_psi2;				fin.getline(dummy, NCHAR);
	/*fin >> par_per_x >> par_per_y >>par_per_z;	fin.getline(dummy, NCHAR);
	fin >> per_xp >> per_xn;			fin.getline(dummy, NCHAR);
	fin >> per_yp >> per_yn;			fin.getline(dummy, NCHAR);
	fin >> per_zp >> per_zn;			fin.getline(dummy, NCHAR);
	                                      		fin.getline(dummy, NCHAR);
	fin >> fre_backup;                        	fin.getline(dummy, NCHAR);
	fin >>mode_backup_ini;                		fin.getline(dummy, NCHAR);
							fin.getline(dummy, NCHAR);
	fin >> decbin;					fin.getline(dummy, NCHAR);
							fin.getline(dummy, NCHAR);
	fin >> pressure_change >>pre_chan_pb;		fin.getline(dummy, NCHAR);
	fin >> pre_chan_1>> pre_chan_pn1 >> pre_chan_pp1>>pre_chan_f1;	fin.getline(dummy, NCHAR);
	fin >> pre_chan_2>> pre_chan_pn2 >> pre_chan_pp2>>pre_chan_f2;	fin.getline(dummy, NCHAR);
	fin >> pre_chan_3>> pre_chan_pn3 >> pre_chan_pp3>>pre_chan_f3;	fin.getline(dummy, NCHAR);
	fin >> pre_chan_4>> pre_chan_pn4 >> pre_chan_pp4>>pre_chan_f4;	fin.getline(dummy, NCHAR);
	fin >> pre_chan_5>> pre_chan_pn5 >> pre_chan_pp5>>pre_chan_f5;	fin.getline(dummy, NCHAR);
	fin >> bodyforce_apply;					fin.getline(dummy, NCHAR);
	fin.getline(dummy, NCHAR);        
	
	//=============CAPILLARY PRESSUR======                                                                                                                
	fin >> pressure_change2 >> pre_chan_pb2 >> interval_pre;					fin.getline(dummy, NCHAR);
	fin >> pre_chan_pns >> pre_chan_pps >> pre_chan_fs >>chan_no >> sat_cri;                 fin.getline(dummy, NCHAR);
	fin >> sat_cri_d;                                                                                fin.getline(dummy, NCHAR);
	//=======================================================================
	
	
	//=======================Least Square Fitting====================
	fin.getline(dummy, NCHAR);
	fin >> least_square >> num_least_square;                          fin.getline(dummy, NCHAR);
	//=======================================================
	
	
	//=======================Rel_Perm_Imb_Drai====================
	fin.getline(dummy, NCHAR);
	fin >>rel_perm_id_dir>>rel_perm_chan_num;       fin.getline(dummy, NCHAR);
	fin >>rel_perm_ift >> vary_ift;       		fin.getline(dummy, NCHAR);
	//=============================================================
	

	//================INPUT UPDATE===========================
	fin >> input_dynamic;           		fin.getline(dummy, NCHAR);
	//=======================================================
	
	
	*/


	fin.close();
	
	
	}
	MPI_Bcast(&n_max,1,MPI_INT,0,MPI_COMM_WORLD);
	MPI_Bcast(&gx,1,MPI_DOUBLE,0,MPI_COMM_WORLD);
	MPI_Bcast(&gy,1,MPI_DOUBLE,0,MPI_COMM_WORLD);MPI_Bcast(&gz,1,MPI_DOUBLE,0,MPI_COMM_WORLD);
	MPI_Bcast(&ContactAngle_parameter,1,MPI_DOUBLE,0,MPI_COMM_WORLD);

	MPI_Bcast(&CapA,1,MPI_DOUBLE,0,MPI_COMM_WORLD);
	MPI_Bcast(&Permeability,1,MPI_DOUBLE,0,MPI_COMM_WORLD);
	MPI_Bcast(&freRe,1,MPI_INT,0,MPI_COMM_WORLD);
	MPI_Bcast(&freVe,1,MPI_INT,0,MPI_COMM_WORLD);MPI_Bcast(&freDe,1,MPI_INT,0,MPI_COMM_WORLD);
	MPI_Bcast(&frePsi,1,MPI_INT,0,MPI_COMM_WORLD);
	MPI_Bcast(&rel_perm_psi,1,MPI_DOUBLE,0,MPI_COMM_WORLD);MPI_Bcast(&rel_perm_psi2,1,MPI_DOUBLE,0,MPI_COMM_WORLD);

	cout<<rel_perm_psi<<endl;

	gxs=gx;gys=gy;gzs=gz;
	psi_solid=ContactAngle_parameter;


}


void Partition_Solid(int*** Solid)
{


int rank = MPI :: COMM_WORLD . Get_rank ();
int para_size=MPI :: COMM_WORLD . Get_size ();
	MPI_Comm comm;
	MPI_Comm_dup(MPI_COMM_WORLD, &comm);

	int sum=0;
	int sum3,sum_rec;
	int ii,jj,kk;
	int mesh_par=para_size;

	int nx=NX+1;
	int ny=NY+1;
	int nz=NZ+1;

	int* rbuf_result;

	for(int k=0 ; k<nz ; k++)				
	for(int j=0 ; j<ny ; j++)
	for(int i=0 ; i<nx ; i++)				
	

	
		{	
			
			
			
			
			if (Solid[i][j][k] == 0)	{sum++;Solid[i][j][k] = sum-1;}
			else
				{Solid[i][j][k] = -1;}
			
		
			
			
		}
	
	sum_rec=sum;
	



	idx_t* vtxdist=NULL;
	idx_t *xadj, *adjncy;
        vtxdist = new idx_t[para_size+1];
		


        int ls1,ls2;
        int *proc_size;
        proc_size = new int[para_size];	
        
        ls1=sum_rec%para_size;
        ls2=(int)sum_rec/para_size;
        
        for (int i=0;i<para_size;i++)
                proc_size[i]=ls2;
        for(int i=0;i<ls1;i++)
                proc_size[i]++;
        
    


        
        vtxdist[0]=0;
        for (int i=1;i<=para_size;i++)
        {        vtxdist[i]=vtxdist[i-1]+proc_size[i-1];
                
        }
        
        
        
        xadj = new idx_t[proc_size[rank]+1];
        sum=0;
        
        
        for(int k=0 ; k<nz ; k++)			
	for(int j=0 ; j<ny ; j++)
	for(int i=0 ; i<nx ; i++)	
	{
	        if ((Solid[i][j][k]>=vtxdist[rank]) and (Solid[i][j][k]<vtxdist[rank+1]))
	                for (int ls=1;ls<19;ls++)
		{
		ii=i+e[ls][0];
		jj=j+e[ls][1];
		kk=k+e[ls][2];	
		
		
		//================
		/*
		if (ii>=nx) ii=0;
		if (ii<0) ii=nx-1;
		if (jj>=ny) jj=0;
		if (jj<0) jj=ny-1;
		if (kk>=nz) kk=0;
		if (kk<0) kk=nz-1;
		*/
		//================
		
		if ((ii>=0) and (ii<nx) and (jj>=0) and (jj<ny) and (kk>=0) and (kk<nz) and (Solid[ii][jj][kk]>=0))
			sum++;
		}
		
	}
        
      adjncy = new idx_t[sum];
      xadj[0]=0;
      
      sum=0;sum3=0;
      for(int k=0 ; k<nz ; k++)			
	for(int j=0 ; j<ny ; j++)
	for(int i=0 ; i<nx ; i++)	
	{
	        if ((Solid[i][j][k]>=vtxdist[rank]) and (Solid[i][j][k]<vtxdist[rank+1]))
	        {
	                for (int ls=1;ls<19;ls++)
	                {
		ii=i+e[ls][0];
		jj=j+e[ls][1];
		kk=k+e[ls][2];	
		
		
		//================
		/*
		if (ii>=nx) ii=0;
		if (ii<0) ii=nx-1;
		if (jj>=ny) jj=0;
		if (jj<0) jj=ny-1;
		if (kk>=nz) kk=0;
		if (kk<0) kk=nz-1;
		*/
		//================
		
		if ((ii>=0) and (ii<nx) and (jj>=0) and (jj<ny) and (kk>=0) and (kk<nz) and (Solid[ii][jj][kk]>=0))
		        {adjncy[sum]=Solid[ii][jj][kk];sum++;}
		        }
		        
		        
		        
		sum3++;
		xadj[sum3]=sum;
		
		
		}
		
	}


	

	

	idx_t *vwgt=NULL;
	
	idx_t *adjwgt=NULL;
	idx_t wgtflag=0;
	idx_t numflag=0;
	idx_t ncon=1;
	idx_t nparts=mesh_par;
	real_t *tpwgts;
	real_t *ubvec;
	idx_t options[10];
	idx_t edgecut;
	idx_t *part;
	part = new idx_t[proc_size[rank]];
	//vwgt = new idx_t[proc_size[rank]*ncon];

	
	tpwgts = new real_t[ncon*nparts];
	for (int i=0;i<ncon*nparts;i++)
	        tpwgts[i]=1.0/(real_t)nparts;
	
	ubvec = new real_t[ncon];
	for (int i=0;i<ncon;i++)
	        ubvec[i]=1.05;
	
	//vwgt = new idx_t[proc_size[rank]];
	//	for (int i=0;i<proc_size[rank];i++)
	//	vwgt[i]=10;

	//adjwgt = new idx_t[sum];
	//	for (int i=0;i<sum;i++)
	//	adjwgt[i]=1;

	
	options[0] = 0;
	
	if (rank==0)
	cout<<"Start Partition"<<endl;
	


	MPI_Barrier(MPI_COMM_WORLD);
	

	
      ParMETIS_V3_PartKway(vtxdist, xadj, adjncy, vwgt,adjwgt, &wgtflag, &numflag, &ncon, &nparts, tpwgts, ubvec, options, &edgecut, part, &comm);

	if (rank==0)
	cout<<"Partition Complete"<<endl;

	int* part_int;
	int* disp;
	int* sta_parts;

	part_int = new int[proc_size[rank]];

	for (int i=0;i<proc_size[rank];i++)
		part_int[i]=part[i];

	if (rank==0)
		rbuf_result = new int[sum_rec];
	
	disp = new int[para_size];
	for (int i=0;i<para_size;i++)
		disp[i]=vtxdist[i];



	MPI_Gatherv(part_int,proc_size[rank],MPI_INT,rbuf_result,proc_size,disp,MPI_INT,0,MPI_COMM_WORLD);

	if (rank==0)
	{
	sta_parts = new int[mesh_par];
	for (int i=0;i<mesh_par;i++)
		sta_parts[i]=0;

	sum=0;
	for (int k=0;k<nz;k++)
	for (int j=0;j<ny;j++)
	for (int i=0;i<nx;i++)
		if (Solid[i][j][k]>=0)
			{Solid[i][j][k]=rbuf_result[sum]+1;sta_parts[rbuf_result[sum]]++;sum++;}
		else	
			{Solid[i][j][k]=0;}

	
	for (int i=0;i<mesh_par;i++)
		cout<<sta_parts[i]<<"	"<<i<<"		"<<sta_parts[i]-(int)sum/mesh_par<<"	"<<(double)(sta_parts[i]-(int)sum/mesh_par)/sta_parts[i]<<endl;
	}

	MPI_Bcast(Solid[0][0],nx*ny*nz,MPI_INT,0,MPI_COMM_WORLD);

	
	delete [] proc_size;
	delete [] xadj;
	delete [] adjncy;
	delete [] part;
	delete [] tpwgts;
	delete [] ubvec;
	delete [] part_int;
	delete [] disp;

	if (rank==0)
		delete [] rbuf_result;

	
	
}

void psi_mixture_ini(float* Psi_rank0)
{
        
        int rank = MPI :: COMM_WORLD . Get_rank ();
        int para_size=MPI :: COMM_WORLD . Get_size ();
        
        int nx=NX+1;
	int ny=NY+1;
	int nz=NZ+1;
	int fluid_sum=0;

        int*** Solid_in;
        
	
	
int nw_node;
int nei_ver;
int lsi,lsj,lsk;
int cori,corj,cork;
int nwn_mark=0;
int cor_i=mix_psi_thickness;
int rad_max,rad1,cenx,ceny,cenz;

cenx=(int)(nx/2);
ceny=(int)(ny/2);
cenz=(int)(nz/2);

rad_max=(int)(ny/2);
if (nz/2>rad_max)
	rad_max=(int)(nz/2);




if (rank==0)
{

     
        for (int i=0;i<nx;i++)
                for (int j=0;j<ny;j++)
                for (int k=0;k<nz;k++)
                if (Solid[i][j][k]==0)
                        Solid2[i][j][k]=1;
                else
                        Solid2[i][j][k]=0;


for (int j=0;j<ny;j++)
        for (int k=0;k<nz;k++)
        if (Solid2[cor_i][j][k]==0)
                fluid_sum++;
        
nw_node = (int)(fluid_sum*protion_w);






if (Solid2[cor_i][ceny][cenz]==0)
        {
                        nei_ver=0;
                        for (int ls=0;ls<18;ls++)
                                {
                                      lsi=cor_i+e[ls][0];
                                      lsj=ceny+e[ls][1];
                                      lsk=cenz+e[ls][2];
                                      if ((lsi>=0) and (lsi<nx) and (lsj>=0) and (lsj<ny) and (lsk>=0) and (lsk<nz)) 
                                              if (Solid2[lsi][lsj][lsk]==1)
                                                        nei_ver=1;
                                }
                        if ((nei_ver==1) and (nwn_mark<nw_node))
                                Solid2[cor_i][ceny][cenz]=2,nwn_mark++;
                        
        }


rad1=1;
while ((nwn_mark<nw_node) and (rad1<rad_max))
{
	for (int k=cenz-rad1;k<=cenz+rad1;k=k+rad1*2)
	for (int j=ceny-rad1;j<=ceny+rad1;j++)
	if (Solid2[cor_i][j][k]==0)
        {
                        nei_ver=0;
                        for (int ls=0;ls<18;ls++)
                                {
                                      lsi=cor_i+e[ls][0];
                                      lsj=j+e[ls][1];
                                      lsk=k+e[ls][2];
                                      if ((lsi>=0) and (lsi<nx) and (lsj>=0) and (lsj<ny) and (lsk>=0) and (lsk<nz)) 
                                              if (Solid2[lsi][lsj][lsk]==1)
                                                        nei_ver=1;
                                }
                        if ((nei_ver==1) and (nwn_mark<nw_node))
                                Solid2[cor_i][j][k]=2,nwn_mark++;
                        
        }


	for (int j=ceny-rad1;j<=ceny+rad1;j=j+rad1*2)
	for (int k=cenz-rad1;k<=cenz+rad1;k++)
	if (Solid2[cor_i][j][k]==0)
        {
                        nei_ver=0;
                        for (int ls=0;ls<18;ls++)
                                {
                                      lsi=cor_i+e[ls][0];
                                      lsj=j+e[ls][1];
                                      lsk=k+e[ls][2];
                                      if ((lsi>=0) and (lsi<nx) and (lsj>=0) and (lsj<ny) and (lsk>=0) and (lsk<nz)) 
                                              if (Solid2[lsi][lsj][lsk]==1)
                                                        nei_ver=1;
                                }
                        if ((nei_ver==1) and (nwn_mark<nw_node))
                                Solid2[cor_i][j][k]=2,nwn_mark++;
                        
        }

	rad1++;
	

        }
        cout<<"Portition of Wetting Fluid "<<(double)nwn_mark/fluid_sum<<endl;



		//---------level2----------------
rad1=1;
while ((nwn_mark<nw_node) and (rad1<rad_max))
{
	for (int k=cenz-rad1;k<=cenz+rad1;k=k+rad1*2)
	for (int j=ceny-rad1;j<=ceny+rad1;j++)
	if (Solid2[cor_i][j][k]==0)
        {
                        nei_ver=0;
                        for (int ls=0;ls<18;ls++)
                                {
                                      lsi=cor_i+e[ls][0];
                                      lsj=j+e[ls][1];
                                      lsk=k+e[ls][2];
                                      if ((lsi>=0) and (lsi<nx) and (lsj>=0) and (lsj<ny) and (lsk>=0) and (lsk<nz)) 
                                              if (Solid2[lsi][lsj][lsk]>0)
                                                        nei_ver=1;
                                }
                        if ((nei_ver==1) and (nwn_mark<nw_node))
                                Solid2[cor_i][j][k]=-2,nwn_mark++;
                        
        }


	for (int j=ceny-rad1;j<=ceny+rad1;j=j+rad1*2)
	for (int k=cenz-rad1;k<=cenz+rad1;k++)
	if (Solid2[cor_i][j][k]==0)
        {
                        nei_ver=0;
                        for (int ls=0;ls<18;ls++)
                                {
                                      lsi=cor_i+e[ls][0];
                                      lsj=j+e[ls][1];
                                      lsk=k+e[ls][2];
                                      if ((lsi>=0) and (lsi<nx) and (lsj>=0) and (lsj<ny) and (lsk>=0) and (lsk<nz)) 
                                              if (Solid2[lsi][lsj][lsk]>0)
                                                        nei_ver=1;
                                }
                        if ((nei_ver==1) and (nwn_mark<nw_node))
                                Solid2[cor_i][j][k]=-2,nwn_mark++;
                        
        }

	rad1++;
	

}


for (int j=0;j<ny;j++)
        for (int k=0;k<nz;k++)
        if (Solid2[cor_i][j][k]==-2)
                Solid2[cor_i][j][k]=2;
        
     cout<<"Portition of Wetting Fluid2 "<<(double)nwn_mark/fluid_sum<<endl;    



     for (int k=0;k<nz;k++)
	for (int j=0;j<ny;j++)
	for (int i=0;i<nx;i++)
		//out4<<Solid[i][j][k]<<"	";
		
		if ((i<=mix_psi_thickness) and (Solid2[mix_psi_thickness][j][k]==0))
				Psi_rank0[i*(NY+1)*(NZ+1)+j*(NZ+1)+k]=-1;
			else
				Psi_rank0[i*(NY+1)*(NZ+1)+j*(NZ+1)+k]=1;
			
			cout<<endl;
			cout<<"PSI RANK0 initialization complete"<<rank<<endl;
			cout<<endl;
}	
	
}
