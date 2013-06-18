// 8980.cpp : Defines the entry point for the console application.
//


#include <map>
#include<direct.h>
#include<stdio.h>
#include <stdlib.h>
#include<iostream>
#include <string.h>
#include<vector>
#include<math.h>
#include <complex>
using namespace std;
#define PI 3.14159

float g_arr[12] = {-0.9,-0.7,-0.5,-0.3,0,0.3,0.5,0.7,0.9,0.95,0.99,-1};
	float alph_arr[12] = {0.01,0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,0.99,-1}; //check no of entries;
	float theta_i_arr[9] = {0,15,30,45,60,70,80,88,-1};
	float rad_theta_i_arr[9] = {0.0,0.261799,0.523599,0.785398,1.047198,1.221730,1.396263,1.535890,-1};
	float r_arr[13] = {0.01,0.05,0.1,0.2,0.4,0.6,0.8,1.0,2.0,4.0,8.0,10.0,-1}; //mfp propotional ..somehow interpolate this properly?
	float theta_s_arr[14] = { 0,0.261799,0.523599,0.785398,1.047198,1.308997,1.570796,1.832596,2.094395,2.356194,2.617994,2.879793,3.141593,-1};

typedef struct{
	float x,y,z;
}Vector;
typedef struct {
  float l_ax, l_ax2; // a+, a-
  float l_ay, l_ay2; // b+, b-
  float l_px, l_py; // k_e, k_c
} FitVars;
typedef struct {
  float ti, g, r, ts, a; // useful information
  Vector maxdir; // w_peak
  Vector proj_grad; // s (plane of symmetry)
  float scale;      // k_s
  FitVars fit_vars;
} FitData;
struct Colortype{

  float rgb[3];
  };
typedef struct {
  float ti, g, r, ts, a,n; // useful information

}dataInput;
#define ASSIGN(name) for (map<char*, float>::iterator it = param_map.begin(); it != param_map.end(); it++) { if (!strcmp(#name, it->first)) { fd.name = it->second; break; } }

dataInput userInputData;




/*********************************************************************************/
/*			Function declarations start here									*/
/*																				*/
/*																				*/
/*																				*/
/*********************************************************************************/

int find_index(float* value_array,float value)
{
	int i =0;
	while(value_array[i] != -1)
	{

	//	printf("value array val %f %f \n",value_array[i],value);
		if(value_array[i] < value + 0.001 && value_array[i] > value - 0.001)
		{
	//		printf("break!\n");
			break;
		}

		i++;
	}

	return i;
}
void print_thetar_3d(FitData ***theta_r_3d)
{
	for(int i=0;i<8;i++) //size of thetai
		for(int j=0;j<12;j++) //size of r
			for(int k=0;k<13;k++) //size of thetas
			{
				printf("ti  = %f  ts = %f  r = %f \n",theta_r_3d[i][j][k].ti,theta_r_3d[i][j][k].ts,theta_r_3d[i][j][k].r);
			}
			getchar();
}
void find_closest(float *param_arr,float user_val,int* lindx_val_ptr,int *rindx_val_ptr)
{
	float prev_val= -1;
	float next_val = -1;
	
	int i=0;
		while(param_arr[i] != -1)//g index
		{
			//prev_a = alpha_eta_space[i][
			prev_val = param_arr[i];
			next_val = param_arr[i+1];
			if(user_val == prev_val || user_val == next_val)
			{
				//printf("value is equal to one of the values existing!"); //found the value , simply substitute , no need to interpolate. decide how to do this
				if(user_val== prev_val)
				{
					*lindx_val_ptr = i;
					*rindx_val_ptr = i;
				}
				else
				{
					*lindx_val_ptr = i+1;
					*rindx_val_ptr = i+1;
				}
			}
			else if(user_val > prev_val && user_val < next_val )
			{
				//printf("value found between %f and %f ",prev_g,next_g);//found something
				*lindx_val_ptr = i;
				*rindx_val_ptr = i+1;
				break;
			}
			else
			{}
			//	printf("value not found !\n");
			i++;
		}
}
float calculate_3d_distance(float x,float y,float z,float ux,float uy,float uz)
{
	float dist = sqrt(pow((x-ux),2)+pow(y-uy,2)+pow(z-uz,2));
	return dist;
}
FitData lerp_thetar_3d(dataInput user_input,FitData*** thetar_3d)
{
	float user_ti = user_input.ti;
	float user_ts = user_input.ts;
	float user_r = user_input.r;

	int ti_lindx = 0;
	int ti_rindx =0;
	int ts_lindx =0;
	int ts_rindx = 0;
	int r_lindx = 0;
	int r_rindx = 0;
	find_closest(rad_theta_i_arr,user_input.ti,&ti_lindx,&ti_rindx);
	find_closest(theta_s_arr,user_input.ts,&ts_lindx,&ts_rindx);
	find_closest(r_arr,user_input.r,&r_lindx,&r_rindx);
	
	//lefttopf1, righttopf1 , rightbottomf1,leftbottomf1
	//lefttopf2, righttopf2 , rightbottomf2,leftbottomf2
	int ti_n[2] = { ti_lindx,ti_rindx};
	int ts_n[2] = { ts_lindx,ts_rindx};
	int r_n[2] = {r_lindx,r_rindx};
	float d[8];
	int counter=0;
	FitData inter_value;
	memset(&inter_value,0,sizeof(FitData));
	float dsum = 0;


	//calculate distances and assign weights 

	for(int ti=0;ti<2;ti++)
	{
		for(int r=0;r<2;r++)
		{
			for(int ts=0;ts<2;ts++)
			{

				//parameters to the function are , when ti = 0 , r = 0 , and ts = 0 , the left most points in the space
				// and so on for each index
				//shorter code. :)
				int indx = counter;
				d[counter] = calculate_3d_distance(thetar_3d[ti_n[ti]][r_n[r]][ts_n[ts]].ti,
													 thetar_3d[ti_n[ti]][r_n[r]][ts_n[ts]].r,
													 thetar_3d[ti_n[ti]][r_n[r]][ts_n[ts]].ts,
													 user_ti,user_r,user_ts);		

				if(d[counter]>0)
					d[counter] = 1.0/d[counter];
				else
					d[counter] = 1;

				dsum += d[counter];

				counter++;
			}
		}
	}
	 //normalize weights  here 
	
	for(int i=0;i<8;i++)
	{
		d[i] = d[i]/dsum;
	}

	
	//calculate the interpolated values here -

	counter = 0 ;

	for(int ti=0;ti<2;ti++)
	{
		for(int r=0;r<2;r++)
		{
			for(int ts=0;ts<2;ts++)
			{

				//parameters to the function are , when ti = 0 , r = 0 , and ts = 0 , the left most points in the space
				// and so on for each index
				//shorter code. :)
				int indx = counter;
				//d[counter] = calculate_3d_distance(thetar_3d[ti_n[ti]][r_n[r]][ts_n[ts]].ti,
										//			 thetar_3d[ti_n[ti]][r_n[r]][ts_n[ts]].r,
										//			 thetar_3d[ti_n[ti]][r_n[r]][ts_n[ts]].ts,
										//			 user_ti,user_r,user_ts);
				
				inter_value.fit_vars.l_ax += d[counter]*thetar_3d[ti_n[ti]][r_n[r]][ts_n[ts]].fit_vars.l_ax;
				inter_value.fit_vars.l_ax2 += d[counter]*thetar_3d[ti_n[ti]][r_n[r]][ts_n[ts]].fit_vars.l_ax2;
				inter_value.fit_vars.l_ay += d[counter]*thetar_3d[ti_n[ti]][r_n[r]][ts_n[ts]].fit_vars.l_ay;
				inter_value.fit_vars.l_ay2 += d[counter]*thetar_3d[ti_n[ti]][r_n[r]][ts_n[ts]].fit_vars.l_ay2;
				inter_value.fit_vars.l_px += d[counter]*thetar_3d[ti_n[ti]][r_n[r]][ts_n[ts]].fit_vars.l_px;
				inter_value.fit_vars.l_py += d[counter]*thetar_3d[ti_n[ti]][r_n[r]][ts_n[ts]].fit_vars.l_py;
				inter_value.scale += d[counter]*thetar_3d[ti_n[ti]][r_n[r]][ts_n[ts]].scale;
				inter_value.proj_grad.x+= d[counter]*thetar_3d[ti_n[ti]][r_n[r]][ts_n[ts]].proj_grad.x;
				inter_value.proj_grad.y+= d[counter]*thetar_3d[ti_n[ti]][r_n[r]][ts_n[ts]].proj_grad.y;
				inter_value.proj_grad.z+= d[counter]*thetar_3d[ti_n[ti]][r_n[r]][ts_n[ts]].proj_grad.z;
				inter_value.maxdir.x += d[counter]*thetar_3d[ti_n[ti]][r_n[r]][ts_n[ts]].maxdir.x;
				inter_value.maxdir.y += d[counter]*thetar_3d[ti_n[ti]][r_n[r]][ts_n[ts]].maxdir.y;
				inter_value.maxdir.z += d[counter]*thetar_3d[ti_n[ti]][r_n[r]][ts_n[ts]].maxdir.z;
				inter_value.ti = user_input.ti;
				inter_value.ts = user_input.ts;
				inter_value.r = user_input.r;
				inter_value.a = user_input.a;
				inter_value.g = user_input.g;
				

				
				counter++;
			}
		}
	}

	//interpolate with 8 neighbors (entammo!)


	return inter_value;
}

FitData lerp_ts_ti_r(dataInput user_input,FitData **lerped_val)
{

	int lerp_size = 156;
	int ti_length = 8;
	float user_ts = user_input.ts;
	float user_ti = user_input.ti;
	float user_r  = user_input.r;

	FitData new_lerp_val;
	FitData*** thetar_3d = new FitData**[8]; //size of theta_i
	for(int i=0;i<13;i++)
	{
		thetar_3d[i] = new FitData*[13]; //size of r 
		for(int j=0;j<13;j++)
		{
			thetar_3d[i][j] = new FitData[13]; //size of theta_s
		}
	}


	//3d coord system consisting of ts , ti and r 
	//reorganize lerp_values //no need to store all data , jsut ts , ti , r coords
	int r_indx=0;
	int ts_indx=0;
	int ti_indx=0;
	for(int ti_index=0;ti_index<ti_length;ti_index++)
	{
		

		for(int rts_index=0;rts_index<lerp_size;rts_index++)
		{
		//find closest values in 3-D
	///		printf("values are -- r = %f  ts= %f  ti=%f\n",lerped_val[j][i].r,lerped_val[j][i].ts,lerped_val[j][i].ti);
			ti_indx = find_index(rad_theta_i_arr,lerped_val[ti_index][rts_index].ti);
			r_indx = find_index(r_arr,lerped_val[ti_index][rts_index].r);
			ts_indx = find_index(theta_s_arr,lerped_val[ti_index][rts_index].ts);

		//	printf("actual value -- %f = ti \n",lerped_val[ti_index][rts_index].ti);
		//	printf("%f = ts \n",thetar_3d[ti_indx][r_indx][ts_indx].r);
		//	printf("actual tindx value is %d \n",ti_index);
		//	printf("ti_indx = %d r_indx = %d ts_indx = %d \n",ti_indx,r_indx,ts_indx);
			memcpy(&thetar_3d[ti_indx][r_indx][ts_indx], &lerped_val[ti_index][rts_index],sizeof(FitData));
		//	printf("copied value in 3d --- %f=ti \n",thetar_3d[ti_indx][r_indx][ts_indx].ti);
		}
		//printf("outside loop -- tindex = %d \n" ,ti_index);
		//printf(" -- end of one ---\n");
		//getchar();

		//getchar();
	}
	//print_thetar_3d(thetar_3d);
	FitData actual_data = lerp_thetar_3d(user_input,thetar_3d);

	
	//getchar();
	//getchar();
	//FitData dummy_fd;
	return actual_data;
	//get all the values in an arry ?..

}

typedef struct g_alpha_eta_space {
		FitData tstir[156]; // each line will have the fit data values associated with it.
		//we willl take each 156 of these values and interpolate !
	}GALPHAETASPACE;


GALPHAETASPACE ***alpha_eta_space ;

	
//#define ASSIGN(name) for (map::iterator it = param_map.begin(); it != param_map.end(); it++) { if (!strcmp(#name, it->first)) { fd.name = it->second; break; } }

struct cmp_str
{
   bool operator()(char const *a, char const *b)
   {
      return std::strcmp(a, b) < 0;
   }
};
void assign_value(FitData *temp_fit_data,char* key,float value)
{
	if(!strcmp(key,"a"))
	{
		temp_fit_data->a = value;
	}
	else if(!strcmp(key,"g"))
	{
		temp_fit_data->g = value;
	}
	else if(!strcmp(key,"r"))
	{
		temp_fit_data->r = value;
	}
	else if(!strcmp(key,"ts"))
	{
		temp_fit_data->ts = value;
	}
	else if(!strcmp(key,"ti"))
	{
		temp_fit_data->ti = value;
	}
	else if(!strcmp(key,"scale"))
	{
		temp_fit_data->scale = value;
	//	printf("%f scale !\n",temp_fit_data->scale);

	}
	else if(!strcmp(key,"maxdir.x"))
	{
		temp_fit_data->maxdir.x = value;
	}
	else if(!strcmp(key,"maxdir.y"))
	{
		temp_fit_data->maxdir.y = value;
	}
	else if(!strcmp(key,"maxdir.z"))
	{
		temp_fit_data->maxdir.z = value;
	}
	else if(!strcmp(key,"proj_grad.x"))
	{
		temp_fit_data->proj_grad.x = value;
	}
	else if(!strcmp(key,"proj_grad.y"))
	{
		temp_fit_data->proj_grad.y = value;
	}
	else if(!strcmp(key,"proj_grad.z"))
	{
		temp_fit_data->proj_grad.z = value;
	}
	else if(!strcmp(key,"fit_vars.l_ax"))
	{
		temp_fit_data->fit_vars.l_ax = value;
	}
	else if(!strcmp(key,"fit_vars.l_ay"))
	{
		temp_fit_data->fit_vars.l_ay = value;
	}
	else if(!strcmp(key,"fit_vars.l_ax2"))
	{
		temp_fit_data->fit_vars.l_ax2 = value;
	}
	else if(!strcmp(key,"fit_vars.l_ay2"))
	{
		temp_fit_data->fit_vars.l_ay2 = value;
	}
	else if(!strcmp(key,"fit_vars.l_px"))
	{
		temp_fit_data->fit_vars.l_px = value;
	}
	else if(!strcmp(key,"fit_vars.l_py"))
	{
		temp_fit_data->fit_vars.l_py = value;
	}
	//fitvars.l_ax  fitvar.l_px py

}
void print_fit_vars(FitData fitdata)
{
	//printf(" %f %f %f %f %f\n",fitdata.a,fitdata.g,fitdata.r,fitdata.ti,fitdata.ts);
	printf(" -- a + = %f ",fitdata.fit_vars.l_ax);
	printf("\t -- a - = %f",fitdata.fit_vars.l_ax2);
	printf("\t -- b+ = %f",fitdata.fit_vars.l_ay);
	printf("\t -- b - = %f",fitdata.fit_vars.l_ay2);
	printf("\t -- Ks = %f",fitdata.scale);
	printf("\t -- Kc = %f",fitdata.fit_vars.l_px);
	printf("\t -- Ke = %f",fitdata.fit_vars.l_py);
}
void populate_data_matrix(FILE* temp_f,int& global_counter,int theta_index,int g_index,int alb_index,GALPHAETASPACE ***alpha_eta_space_ptr)
{
	int num_entries; // number of entries in this file
							fread(&num_entries, sizeof(int), 1, temp_f);
							//printf("num of entries=%d\nint size=%d float size=%d char size=%d\n",num_entries,sizeof(int),sizeof(float),sizeof(char));

							//read each file  , based on number of lines / entries
							map<char*, float,cmp_str> param_map;
							FitData temp_fitdata;
							for (int q=0; q<num_entries; q++) { //, bin++
							  param_map.clear();
							  int colno = 0;

							  //read till end of file is reached
							  while (!feof(temp_f)) {
								int len=0;
								fread(&len, sizeof(int), 1, temp_f);
								if (len == 0)
								  break;
								char* key = (char*)malloc(len+1);
								fread(key, sizeof(char), len, temp_f);
								key[len] = 0;
								if (key[0] < '0') {
								  free(key);
								  continue;
								}
								float val;
								fread(&val, sizeof(float), 1, temp_f);
								//const char* const_key = key;
								param_map[key] = val;
								assign_value(&temp_fitdata,key,val);
								//printf("%s=%f",key,val);
								/*if(!strcmp(key,"r"))
								{
									printf(" r found ! %f %f ",val,param_map["r"]);
									int a;
									cin>>a;
								}*/

								//int a;
								//cin>>a;
								
							  }
						
							  FitData fd;
							  if(true) {
								 // printf("line=%d\n ",q);
													
							  }

							  memcpy(&alpha_eta_space_ptr[theta_index][g_index][alb_index].tstir[q],&temp_fitdata,sizeof(FitData));
							

							  global_counter++;
							}
}

void readFitData(){
	map<char*, float> param_map;
	vector< map<char*, float> > pa2;


	char filename[200];
	float def_r = 0.01;
	float def_thetas = 0.00;
	float def_a = 0.1;

	//for a given r , theta_s , theta_i

	//find the nearest alpha
	//find nearest eta 
	//find the nearest g  , for intermediate values of alpha , eta and g , 
	//interpolate values of all constants for the same (dont interpolate)  theta_i , theta_s, r  . store this in another array
	//insdie the given alpha , g array, eta array. (eta is ssumed to be a constant for now )
	//for this , we dont need to store separately. the interpolation gives the reqd values.
	//after we are done interpolating the alpha  and g values , interpolate b/w the theta_i , s and r values inside the chosen 2d space list.
	//go inside that array , and then find the nearest 
	/** reading sequence of files and storing values */

	
	char iterative_file_name[200];
	

	int no_entries=0;
	int global_counter = 0;
	
	alpha_eta_space = new GALPHAETASPACE**[8];
	
	for(int i=0;i<12;i++)
	{
		alpha_eta_space[i] = new GALPHAETASPACE*[12];
	
	for(int j=0;j<12;j++)
	{
		alpha_eta_space[i][j]= new GALPHAETASPACE[12];
	}

	}
	///printf("size of alphaeta space is %d\n",sizeof(GALPHAETASPACE));
	//printf("size of alphaeta space pointer is %d\n",sizeof(alpha_eta_space));
	//printf("size of fitdata is %d\n",sizeof(FitData));

	printf("\nLoading data please wait .... \n");
	for(int theta_index=0;theta_index<8;theta_index++)
	{
		for(int g_index=0;g_index<11;g_index++)
		{
			for(int alb_index=0;alb_index<11;alb_index++)
			{
				sprintf(iterative_file_name,"ior1.4\\ti%g_g%g_a%g.fit",theta_i_arr[theta_index],g_arr[g_index],alph_arr[alb_index]);
				FILE *temp_f = fopen(iterative_file_name,"rb");
				if(temp_f!=NULL)
				{
					//printf("f-exists!\n");
					no_entries++;
					//printf("entry no %d \n",no_entries);
					/**********************/

					

					populate_data_matrix(temp_f,global_counter,theta_index,g_index,alb_index,alpha_eta_space);
							

					/*********************/


					fclose(temp_f);
				}
				//printf("global_counter value is %d\n",global_counter);
				
			}
		}
	}
	printf("Finished loading data ......\n");
}

FitData  lerp_user_input(dataInput user_input,FitData **lerped_val){

	float user_g = 0; //must be in the range -0.9 - +0.99
	float user_a = 0; //must be in the range 0.01 - +0.99

	bool invalid_input = false;

	
		//printf("Enter desired value for g \n");
		//cin>>user_g;
	//instead of reading from console ,
		user_g = user_input.g ;
		if(user_g <-0.9 || user_g >0.99)
			invalid_input = true;

		if(invalid_input)
		{
			//printf("Please enter valid value for g ");
			//cin>>user_g;
			//invalid_input = false;
			//if(user_g <-0.9 || user_g >0.99)
			//invalid_input = true;
			user_g = 0.99;
		}

		//printf("Enter desired value for a (alpha/albedo) \n");
		//cin>>user_a;
		user_a = user_input.a; //get value from user input.

		if(user_a <=0.01 || user_a >0.99)
			invalid_input = true;

		if(invalid_input)
		{
			//printf("Please enter valid value for a ");
			//cin>>user_a;
			//invalid_input = false;
			user_a = 0.99;
			//if(user_a <0.01|| user_a >0.99)
			invalid_input = true;
		}


		//find the nearest values in the array that match , and then interpolate.
	//alpha_eta_space_ptr[g_index][alb_index].tstir[q]
		float prev_a = -1;
		float next_a = -1;
		float prev_g = -1;
		float next_g = -1;
	//	printf("%d is the size of g_arr\n",sizeof(g_arr));


		//CHECK RANGE IN WHICH THE VALUE FALLS 
		//put this in a  function later
		int lindx_g=0;
		int rindx_g=0;

		for(int i=0;i<10;i++) //g index
		{
			//prev_a = alpha_eta_space[i][
			prev_g = g_arr[i];
			next_g = g_arr[i+1];
			if(user_g == prev_g || user_g == next_g)
			{
				//printf("value is equal to one of the values existing!"); //found the value , simply substitute , no need to interpolate. decide how to do this
				if(user_g == prev_g)
				{
					lindx_g = i;
					rindx_g = i;
				}
				else
				{
					lindx_g = i+1;
					rindx_g = i+1;
				}
			}
			else if(user_g > prev_g && user_g < next_g )
			{
				//printf("value found between %f and %f ",prev_g,next_g);//found something
				lindx_g = i;
				rindx_g = i+1;
				break;
			}
			else
			{}
			//	printf("value not found !\n");
		}

		int lindx_a=0;
		int rindx_a=0;

		for(int i=0;i<10;i++) //a index
		{
			//prev_a = alpha_eta_space[i][
			prev_a = alph_arr[i];
			next_a = alph_arr[i+1];
			if(user_a == prev_a || user_a == next_a)
			{
				//printf("value is equal to one of the values existing!"); //found the value , simply substitute , no need to interpolate. decide how to do this
				if(user_a == prev_a)
					lindx_a = rindx_a = i;
				else
					lindx_a=rindx_a=i+1;
			}
			else if(user_a > prev_a && user_a < next_a )
			{
				//printf("value found between %f and %f ",prev_a,next_a);//found something
				lindx_a = i;
				rindx_a = i+1;
				break;
			}
			else
			{}
				//printf("value not found !\n");
		}

		//END CHECK RANGE IN WHICH VALUE FALLS

		//START LERP 

		

		for(int ti_index = 0 ; ti_index < 8;ti_index++)
		{
		for(int i=0;i<156;i++)
		{
			FitData leftbottom = alpha_eta_space[ti_index][lindx_g][lindx_a].tstir[i] ;
			FitData lefttop = alpha_eta_space[ti_index][lindx_g][rindx_a].tstir[i];
			FitData righttop = alpha_eta_space[ti_index][rindx_g][rindx_a].tstir[i];
			FitData rightbottom = alpha_eta_space[ti_index][rindx_g][lindx_a].tstir[i];

			float d1 = sqrt(pow((lindx_g-user_g),2) + pow((lindx_a-user_a),2));
			float d2 = sqrt(pow((lindx_g-user_g),2) + pow((rindx_a-user_a),2));
			float d3 = sqrt(pow((rindx_g-user_g),2) + pow((rindx_a-user_a),2));
			float d4 = sqrt(pow((rindx_g-user_g),2) + pow((lindx_a-user_a),2));

			
			if(d1==0)
			{
				d1 = 1;
				d2 = 0;
				d3 = 0;
				d4 = 0;
			}
			else if(d2 == 0)
			{
				d1 = 0;
				d2 = 1;
				d3 = 0;
				d4 = 0;
			}
			else if(d3 == 0)
			{
				d1 = 0;
				d2 = 0;
				d3 = 1;
				d4 = 0;
			}
			else if(d4 == 0)
			{
				d1 = 0;
				d2 = 0;
				d3 = 0;
				d4 = 1;

			}
			else{
				d1 = 1/d1;
				d2 = 1/d2;
				d3 = 1/d3;
				d4 = 1/d4;
			}

			float distsum = d1+d2+d3+d4;

			d1 = d1/distsum;
			d2 = d2/distsum;
			d3 = d3/distsum;
			d4 = d4/distsum;

			distsum = d1+d2+d3+d4;

		memcpy(&lerped_val[ti_index][i],&leftbottom,sizeof(FitData));
		lerped_val[ti_index][i].fit_vars.l_ax = leftbottom.fit_vars.l_ax*d1 + lefttop.fit_vars.l_ax*d2 + righttop.fit_vars.l_ax*d3 + rightbottom.fit_vars.l_ax*d4;
		lerped_val[ti_index][i].fit_vars.l_ax2 = leftbottom.fit_vars.l_ax2*d1 + lefttop.fit_vars.l_ax2*d2 + righttop.fit_vars.l_ax2*d3 + rightbottom.fit_vars.l_ax2*d4;
		lerped_val[ti_index][i].fit_vars.l_ay = leftbottom.fit_vars.l_ay*d1 + lefttop.fit_vars.l_ay*d2 + righttop.fit_vars.l_ay*d3+rightbottom.fit_vars.l_ay*d4;
		lerped_val[ti_index][i].fit_vars.l_ay2 = leftbottom.fit_vars.l_ay2*d1 + lefttop.fit_vars.l_ay2*d2 + righttop.fit_vars.l_ay2*d3+rightbottom.fit_vars.l_ay2*d4;
		lerped_val[ti_index][i].fit_vars.l_px = leftbottom.fit_vars.l_px*d1 + lefttop.fit_vars.l_px*d2 + righttop.fit_vars.l_px*d3+rightbottom.fit_vars.l_px*d4;
		lerped_val[ti_index][i].fit_vars.l_py = leftbottom.fit_vars.l_py*d1 + lefttop.fit_vars.l_py*d2 + righttop.fit_vars.l_py*d3+rightbottom.fit_vars.l_py*d4;
		lerped_val[ti_index][i].scale = leftbottom.scale*d1 + lefttop.scale*d2 + righttop.scale*d3+rightbottom.scale*d4;

		//if(lerped_val[i].scale > 0.002)
		//printf(" **************LERPED Value is %f ********************************" ,lerped_val[i].scale);
	

		}
		}
		//END LERP 
		FitData new_lerp_val = lerp_ts_ti_r(user_input,lerped_val);  //2d value separated by theta.....

		return new_lerp_val;
		//return lerped_val[0][0]; // return the first value; //ideally match the rest of the values and pick ?
}


/***********************************************************************************/
void outputFitDate(FitData fd1){
	printf("ks=%g\n",fd1.scale);
	printf("ke=%g\n",fd1.fit_vars.l_px);
	printf("kc=%g\n",fd1.fit_vars.l_py);
	printf("ap=%g\n",fd1.fit_vars.l_ax);
	printf("bp=%g\n",fd1.fit_vars.l_ay);
	printf("an=%g\n",fd1.fit_vars.l_ax2);
	printf("bn=%g\n",fd1.fit_vars.l_ay2);
	printf("wpx=%g\n",fd1.maxdir.x);
	printf("wpy=%g\n",fd1.maxdir.y);

}
FitData inputFitDate(FitData fd1){
	fd1.scale=0.0890;
	fd1.fit_vars.l_px=1;
	fd1.fit_vars.l_py=1.537232;
	fd1.fit_vars.l_ax=0.038048;
	fd1.fit_vars.l_ay=0.029257;
	fd1.fit_vars.l_ax2=0.018472;
	fd1.fit_vars.l_ay2=0.029257;
	fd1.maxdir.x=0.005662;
	fd1.maxdir.y=0.99968;
	fd1.maxdir.z=0.005662 ;
	fd1.proj_grad.x=-0.7071;
	fd1.proj_grad.y=0;
	fd1.proj_grad.z=0.707107;
	return fd1;

}
float degree2Radian(float degree){
	return degree*3.1415926/180;
}
void tranV2F3(float v[],Vector a){
	v[0]=a.x;
	v[1]=a.y;
	v[2]=a.z;
}

float vecLength(float v[]){

	return sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
}

float dotProduct(float v1[],float v2[]){
	float r=0;
	float v1l=vecLength(v1);
	float v2l=vecLength(v2);
	for(int i=0;i<3;i++) 
		r+=v1[i]/v1l*(v2[i]/v2l);
	
	return r;
}

FitData readFit(dataInput userInputData){
	
FitData fd;
fd.a = 0.1;
fd.g = 0.3;
fd.g  = 0.6;
fd.r = 0.66;
fd.scale = 0.0333;

return fd;
}

int colRange=30;

float* imgoutput(int width, int height,  Colortype *pixel[]){

	 float* pixels = new float[width*height*3];
		for(int j=0;j<height;j++)
			for(int i=0;i<width;i++)
			{
				for(int x=0;x<3;x++){

					pixels[(j*width+i)*3+x]=pixel[width-1-i][j].rgb[x];
					//*colRange*2.0-colRange;
				}

			}
		return pixels;
}

void PPMoutput(char *filename, int width, int height,  Colortype *pixel[]){
	FILE *outputPPM;
	outputPPM=fopen(filename,"w");
	if(outputPPM!=NULL){
		fprintf(outputPPM,"%s %d %d %d\n","P3",width,height,255);

		for(int j=height-1;j>=0;j--)
			for(int i=width-1;i>=0;i--)
			{

				fprintf(outputPPM,"%d %d %d\n",(int) pixel[i][j].rgb[0]*255,(int) (pixel[i][j].rgb[1]*255),(int) (pixel[i][j].rgb[2]*255));
			}
	}else {cout<<"can not produce the output image file. \n";perror("Error");exit(0);}
	fclose(outputPPM);


}
void dhsv2rgb(double h, double s, double v, 
              double *r, double *g, double *b)    /*...hexcone model...*/
//double        h, s, v, *r, *g, *b;    /* all variables in range [0,1[ */
/* here, h=.667 gives blue, h=0 or 1 gives red. */
{  /* see Alvy Ray Smith, Color Gamut Transform Pairs, SIGGRAPH '78 */
        int     i;
        double  f, m, n, k;
        //double        modf(), trash;
        float trash;
        h = 6 * modff(h, &trash);
        i = floor(h);
        f = h - i;
        m = (1 - s);
        n = (1 - s * f);
        k = (1 - (s * (1 - f)));
        switch (i) {
        case 0: 
                *r = 1; 
                *g = k; 
                *b = m; 
                break;
        case 1: 
                *r = n; 
                *g = 1; 
                *b = m; 
                break;
        case 2: 
                *r = m; 
                *g = 1; 
                *b = k; 
                break;
        case 3: 
                *r = m; 
                *g = n; 
                *b = 1; 
                break;
        case 4: 
                *r = k; 
                *g = m; 
                *b = 1; 
                break;
        case 5: 
                *r = 1; 
                *g = m; 
                *b = n; 
                break;
        default: 
                fprintf(stderr, "bad i: %f %d", h, i); 
                //exit(1);
                return;
        }
        f = *r;
        if ( f < *g ) 
                f = *g;
        if ( f < *b ) 
                f = *b;
        f = v / f;
        *r *= f;
        *g *= f;
        *b *= f;
}



float* equation8(FitData fd,float n1,int xdim, int  ydim){
	//Ks*exp((-Ke*Mu) - Kc*Ki);
	
	// polar coordinate equations
	//Wpeak has x , y , z directions.
	// check if its normalized.
	//else figure out the R value...assume its one. ?.. print out.
	// calculate theta , phi from it . 
	// project it using the spherical projection thingie.


	//PROJECTION FORMULA ( spherical / disk)

	// Xd = 2*theta0/pi * cos(phi0);
	// Yd = 2*theta0/pi * sin (phi0);

	// theta0 = arcos(wpeak.z/r); // z will  be from wpeak direction
	// phi0 = arctan (wpeak.y/wpeak.x); // y and x will be from wpeakdirection
	//r =  sqrt(wpeak.x^2 + wpeak.y^2 + wpeak.z^2)

	float coord[]={1,0,0};
	float scp[3];
	tranV2F3(scp,fd.proj_grad);
	//tranV2F3(coord,fd.maxdir);
	float maga = 1;
	float magb = sqrt(pow(scp[0],2)+pow(scp[1],2) +pow(scp[2],2));
	float angle=acos(dotProduct(scp,coord)/maga*magb);
	if(angle > PI)
		angle = angle  - (PI/2.0);
	else
	angle = (PI/2.0) - angle;

	//use the origin obtained to offset the origin ... 
	//call it X0 , Y0;
	float xout = fd.r*cos(fd.ts);
	float yout = fd.r*sin(fd.ts);
	//normalize wpeak
	fd.maxdir.x = fd.maxdir.x/sqrt(pow(fd.maxdir.x,2) + pow(fd.maxdir.y,2) + pow(fd.maxdir.z,2));
	fd.maxdir.y = fd.maxdir.y/sqrt(pow(fd.maxdir.x,2) + pow(fd.maxdir.y,2) + pow(fd.maxdir.z,2));
	fd.maxdir.z = fd.maxdir.z/sqrt(pow(fd.maxdir.x,2) + pow(fd.maxdir.y,2) + pow(fd.maxdir.z,2));

	float Xs = fd.maxdir.x + xout;
	float Ys = fd.maxdir.y + yout;
	float Zs = 0;

	float rad  = sqrt(pow(fd.maxdir.x,2) + pow(fd.maxdir.y,2) + pow(fd.maxdir.z,2));
	float theta0 = acos(fd.maxdir.z/rad);
	float phi0 = atan2(fd.maxdir.y,fd.maxdir.x);
	std::cout<<"value of Rad is "<<rad;


	//project it back to the unit disk from spherical coords.
	float X0 = 2*(theta0/PI)*cos(phi0);
	float Y0 = 2*(theta0/PI)*sin(phi0);
	//X0 = fd.maxdir.x;
	//Y0 = fd.maxdir.y;

	//printf("angle=%g\n",angle);
//	int xdim = 200;
//	int ydim =200;
	float max=0,min=0,maxx,maxy;
	float Intensity[500][500];
	for(int ix=0;ix<xdim;ix++){
		for(int iy=0;iy<ydim;iy++){
			float xd0 = (ix-xdim/2.0)/(xdim/2.0);
			float yd0 = (iy-ydim/2.0)/(ydim/2.0);
			float xd = xd0;
			float yd = yd0;
			
			xd = xd + X0;
			yd = yd + Y0;
			xd = xd*cos(angle)- yd*sin(angle);//-fd.maxdir.x;
			yd = xd*sin(angle) + yd*cos(angle);//-fd.maxdir.y;

		//	xd = xd*cos(angle)- yd*sin(angle);//-fd.maxdir.x;
		//	yd = xd*sin(angle) + yd*cos(angle);//-fd.maxdir.y;

		//	xd = xd + fd.r*cos(fd.ts);
		//	yd = yd + fd.r*sin(fd.ts);

			//printf("sin and cos values are %f %f \n",cos(fd.ts),sin(fd.ts));

			
			float a,b;
			if (xd < 0) 
			{
				a= fd.fit_vars.l_ax2;
			}
			else
			{
				a = fd.fit_vars.l_ax;
			}

		    if(yd <0)
			{
				b =fd.fit_vars.l_ay2;
			}
		    else 
			{
					 b = fd.fit_vars.l_ay;
			}
//		    printf("%g %g ",xd,yd);

		    complex<float> i=sqrt( complex<float>(-1) );
		    complex<float> Z= (xd)/a + i*(yd)/b;

		    //acosh(x)=lg(x+sqrt(X*X-1))
		    complex<float> temp=Z*Z;
		    temp-=1.0;
		    Z=log(Z+sqrt(temp));
		   // cout<<real(Z)<<" ";
		    float mu=real(Z);
		    if(mu<0) mu=-mu;
		    float kI=sqrt((xd)*(xd) + (yd)*(yd));

		    //printf("%g ",Z);
		    float Ft=1;
		    float thetai=atan(fd.maxdir.y/fd.maxdir.x);
		    float thetat=atan(yd/xd);
		   // Ft=(cos(thetai)*(1+n1)*(cos(thetai)+cos(thetat))*n1)/(cos(thetai)*cos(thetat)*(1+n1*n1)+n1*(cos(thetai)*cos(thetai)+cos(thetat)*cos(thetat)));
		    Intensity[ix][iy]=fd.scale*exp(-fd.fit_vars.l_px*mu-fd.fit_vars.l_py*kI)*Ft;

		    //printf("%g ",Intensity[ix][iy]);

		    if(max <Intensity[ix][iy])
		                {max = Intensity[ix][iy];maxx=ix;maxy=iy;}
		    if(min >Intensity[ix][iy])
		    			min = Intensity[ix][iy];
			}


			//printf("\n");
	}
	//printf("max=%g %g %g",max,maxx,maxy);

	//color map is right here 

	Colortype *col[500];
	for(int ix=0;ix<xdim;ix++){
		col[ix]=new Colortype[ydim];
			for(int iy=0;iy<ydim;iy++){
				Intensity[ix][iy]/=max;
				//printf("%g ",Intensity[ix][iy]);
//				Intensity[ix][iy]=(Intensity[ix][iy]-min)/(max-min);
			    Colortype tem;
				/*tem.rgb[0]=Intensity[ix][iy];
				tem.rgb[1]=Intensity[ix][iy];
				tem.rgb[2]=1-Intensity[ix][iy];
				//blue to yellow red

				if(Intensity[ix][iy]<0.5){
				tem.rgb[0]=Intensity[ix][iy]*2;
				tem.rgb[1]=Intensity[ix][iy]*2;
				tem.rgb[2]=1-Intensity[ix][iy]*2;
				}else{
					tem.rgb[0]=1;
					tem.rgb[1]=2-Intensity[ix][iy]*2;
					tem.rgb[2]=0;
				}
				*/

				/*if(Intensity[ix][iy]<0.5)
					tem.rgb[0] = 0;
				else
					tem.rgb[0] = 2*Intensity[ix][iy] - 1 ; //contribution should be 1 when intensity = 1 ,contribution should be zero once Intensity > 0.5
				
				if(Intensity[ix][iy]<=0.5)
					tem.rgb[1] =  2*Intensity[ix][iy] ;
				else
				    tem.rgb[1] = 2*(1-Intensity[ix][iy]);

				if(Intensity[ix][iy]>0.5)
					tem.rgb[2] = 0;
				else
					tem.rgb[2] = 1 - 2*Intensity[ix][iy];*/
					
				double rd = 0;
				double gd = 0;
				double bd = 0;
				dhsv2rgb(Intensity[ix][iy],1,1,&rd,&gd,&bd);
				tem.rgb[0] = rd;
				tem.rgb[1]= gd;
				tem.rgb[2] = bd;

			//	tem.rgb[1] = Intensity[ix][iy]; //contribution should be 1 when intensity = 0.5
			//	tem.rgb[2] = Intensity[ix][iy]; //contribution should be 1 when intensity = 1
				//tem.rgb[1]= Intensity[ix][iy]*127;
				//tem.rgb[2] = Intensity[ix][iy]*100;

				//tem.rgb[0] = Intensity[ix][iy]*255;
				//tem.rgb[1] = Intensity[ix][iy]*255;
				
				col[ix][iy]=tem;
			}
			//printf("\n");
	}

	char *outputfile="PPM1.ppm";
	return imgoutput(xdim,ydim,col);
	//PPMoutput(outputfile,xdim,ydim,col);


}




void PPMoutput(dataInput userInput, int width,int height){
	
	FitData** lerped_val = new FitData*[8];

		for(int i=0;i<8;i++)
			lerped_val[i] = new FitData[156];

	FitData fd=lerp_user_input(userInput,lerped_val);  //theta i, g, albego, theta s,r

	

		for(int i=0;i<8;i++)
			delete[] lerped_val[i];

		delete[] lerped_val;
	
	/*userInput.ti=60*3.1415926/180;
	userInput.ts=60*3.1415926/180;
	userInput.r=0.8;
	userInput.a=0.1;
	userInput.g=0.3;
	userInput.n=1.4;*/


	
	print_fit_vars(fd);

	FILE *outputPPM;
		outputPPM=fopen("PPM1.ppm","w");

		if(outputPPM!=NULL){
			float* pixel=equation8(fd,userInput.n,width,height);
			fprintf(outputPPM,"%s %d %d %d\n","P3",width,height,255);

			for(int j=height-1;j>=0;j--)
				for(int i=0;i<width;i++)
				{

					fprintf(outputPPM,"%d %d %d\n",(int) (pixel[(j*width+i)*3+0]*255),(int) (pixel[(j*width+i)*3+1]*255),(int) (pixel[(j*width+i)*3+2]*255));
				}
		}else {cout<<"can not produce the output image file. \n";perror("Error");exit(0);}
		fclose(outputPPM);
		system("explorer PPM1.ppm");

}

float* wholepic(dataInput userInput, int width,int height){
	//this is the main function..
	//reads the fit data ,passes it to the equation.
	FitData** lerped_val = new FitData*[8];

		for(int i=0;i<8;i++)
			lerped_val[i] = new FitData[156];

	FitData fd= lerp_user_input(userInput,lerped_val);  //theta i, g, albego, theta s,r

	for(int i=0;i<8;i++)
	delete[] lerped_val[i];

	delete[] lerped_val;

	FitData dummy_fd ;
	userInput.ti=60*3.1415926/180;
	userInput.ts=60*3.1415926/180;
	userInput.r=0.8;
	userInput.a=0.1;
	userInput.g=0.3;
	userInput.n=1.4;
	dummy_fd.a = userInput.a;
	dummy_fd.g= userInput.g;
	dummy_fd.r = userInput.r;
	dummy_fd.ti = userInput.ti;
	dummy_fd.ts = userInput.ts;
	//printf("%f --- ts value is ",fd.ts);
	//printf("%f --- sints",sin(fd.ts));
	return	equation8(fd,userInput.n,width,height);
}
