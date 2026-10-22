#include "global.h"

//这个是快速排序的代码，网上偷的，好像有点小问题，但是我觉得无伤大雅，就不想改了：
//这里数组的第[0]位是辅助运算位，用来存储基准值的，所以在用这个代码的时候不要用第[0]位，一切从[1]开始哈
int qusort(int s[],u32 start,u32 end)    //自定义函数 qusort()
{
		int i,j;    //定义变量为基本整型
		i=start;    //将每组首个元素赋给i
		j = end;    //将每组末尾元素赋给j
		s[0]=s[start];    //设置基准值
		while(i<j)
		{
				while(i<j&&s[0]<s[j])
				j--;    //位置左移
				if(i<j)
				{
						s[i]=s[j];    //将s[j]放到s[i]的位置上
						i++;    //位置右移
				}
				while(i<j&&s[i]<=s[0])	i++;    //位置左移
				if(i<j)
				{
						s[j]=s[i];    //将大于基准值的s[j]放到s[i]位置
						j--;    //位置左移
				}
		}
		s[i]=s[0];    //将基准值放入指定位置
		if (start<i)	qusort(s,start,j-1);    //对分割出的部分递归调用qusort()函数
		if (i<end)	qusort(s,j+1,end);
		return 0;
}

//tustin continuous 2 discrete coefficient  // QPR控制器的离散化系数
float cy1=(2.0f*PR_Ts*PR_Ts*PR_w_0*PR_w_0 - 8.0f)/(PR_Ts*PR_Ts*PR_w_0*PR_w_0 + 4.0f*PR_w_c*PR_Ts + 4.0f);
float cy2=(PR_Ts*PR_Ts*PR_w_0*PR_w_0 + 4.0f- 4.0f*PR_Ts*PR_w_c )/(PR_Ts*PR_Ts*PR_w_0*PR_w_0 + 4.0f*PR_w_c*PR_Ts + 4.0f);
float cx0=(4.0f*PR_Kp + PR_Ts*PR_Ts*PR_Kp*PR_w_0*PR_w_0 + 4.0f*PR_Ts*PR_Kp*PR_w_c + 4.0f*PR_Ts*PR_Kr*PR_w_c)/(PR_Ts*PR_Ts*PR_w_0*PR_w_0 + 4.0f*PR_w_c*PR_Ts + 4.0f);
float cx1=(2.0f*PR_Kp*PR_Ts*PR_Ts*PR_w_0*PR_w_0 - 8.0f*PR_Kp)/(PR_Ts*PR_Ts*PR_w_0*PR_w_0 + 4.0f*PR_w_c*PR_Ts + 4.0f);
float cx2=(4.0f*PR_Kp+PR_Ts*PR_Ts*PR_Kp*PR_w_0*PR_w_0 - 4.0f*PR_Ts*PR_Kp*PR_w_c - 4.0f*PR_Ts*PR_Kr*PR_w_c)/(PR_Ts*PR_Ts*PR_w_0*PR_w_0 + 4.0f*PR_w_c*PR_Ts + 4.0f);

void AAL_Control_Controller(Controller_Structure *p,float x) // QPR电流控制器的离散化
{
	p->x[0] = x;
	//tustin 离散化PR控制
	p->y[0] =  -(cy1)*(p->y[1]) - cy2*(p->y[2]) + cx0*(p->x[0]) + cx1*(p->x[1]) + cx2*(p->x[2]);
	//限幅代码
	if(p->y[0] > PR_Max) p->y[0]=PR_Max;
	if(p->y[0] < PR_Min) p->y[0]=PR_Min;
  //更新数据
	p->x[3] = p->x[2];
	p->x[2] = p->x[1];
	p->x[1] = p->x[0];

	p->y[3] = p->y[2];
	p->y[2] = p->y[1];
	p->y[1] = p->y[0];
}

float cy1_3=(2.0f*PR_Ts_3*PR_Ts_3*PR_w_0_3*PR_w_0_3 - 8.0f)/(PR_Ts_3*PR_Ts_3*PR_w_0_3*PR_w_0_3 + 4.0f*PR_w_c_3*PR_Ts_3 + 4.0f);
float cy2_3=(PR_Ts_3*PR_Ts_3*PR_w_0_3*PR_w_0_3 + 4.0f- 4.0f*PR_Ts_3*PR_w_c_3)/(PR_Ts_3*PR_Ts_3*PR_w_0_3*PR_w_0_3 + 4.0f*PR_w_c_3*PR_Ts_3 + 4.0f);
float cx0_3=(4.0f*PR_Kp_3 + PR_Ts_3*PR_Ts_3*PR_Kp_3*PR_w_0_3*PR_w_0_3 + 4.0f*PR_Ts_3*PR_Kp_3*PR_w_c_3 + 4.0f*PR_Ts_3*PR_Kr_3*PR_w_c_3)/(PR_Ts_3*PR_Ts_3*PR_w_0_3*PR_w_0_3 + 4.0f*PR_w_c_3*PR_Ts_3 + 4.0f);
float cx1_3=(2.0f*PR_Kp_3*PR_Ts_3*PR_Ts_3*PR_w_0_3*PR_w_0_3 - 8.0f*PR_Kp_3)/(PR_Ts_3*PR_Ts_3*PR_w_0_3*PR_w_0_3 + 4.0f*PR_w_c_3*PR_Ts_3 + 4.0f);
float cx2_3=(4.0f*PR_Kp_3+PR_Ts_3*PR_Ts_3*PR_Kp_3*PR_w_0_3*PR_w_0_3 - 4.0f*PR_Ts_3*PR_Kp_3*PR_w_c_3 - 4.0f*PR_Ts_3*PR_Kr_3*PR_w_c_3)/(PR_Ts_3*PR_Ts_3*PR_w_0_3*PR_w_0_3 + 4.0f*PR_w_c_3*PR_Ts_3 + 4.0f);

void AAL_Control_Controller_no3(Controller_Structure *p,float x) // QPR电流控制器的离散化
{
	p->x[0] = x;
	//tustin 离散化PR控制
	p->y[0] =  -(cy1_3)*(p->y[1]) - cy2_3*(p->y[2]) + cx0_3*(p->x[0]) + cx1_3*(p->x[1]) + cx2_3*(p->x[2]);
	//限幅代码
	if(p->y[0] > PR_Max) p->y[0]=PR_Max;
	if(p->y[0] < PR_Min) p->y[0]=PR_Min;
  //更新数据
	p->x[3] = p->x[2];
	p->x[2] = p->x[1];
	p->x[1] = p->x[0];

	p->y[3] = p->y[2];
	p->y[2] = p->y[1];
	p->y[1] = p->y[0];
}

//SOGI滤波器
void SOGI_Filter_Calc(SOGI_Filter* p)
{
    /*******************************Calc Output*******************************************************/
    p->SOGI_Yn = p->SOGI_C[0] * p->SOGI_X1n + p->SOGI_C[1] * p->SOGI_X2n + p->SOGI_D * p->SOGI_Un;

    /*******************************Calc New State****************************************************/
    p->SOGI_X1n_1 = p->SOGI_A[0][0] * p->SOGI_X1n + p->SOGI_A[0][1] * p->SOGI_X2n + p->SOGI_B[0] * p->SOGI_Un;
    p->SOGI_X2n_1 = p->SOGI_A[1][0] * p->SOGI_X1n + p->SOGI_A[1][1] * p->SOGI_X2n + p->SOGI_B[1] * p->SOGI_Un;

    /*******************************Update State******************************************************/
    p->SOGI_X1n = p->SOGI_X1n_1;
    p->SOGI_X2n = p->SOGI_X2n_1;
}

// 抗饱和PID计算宏定义
void pid_reg_calc(PIDREG *v)
{
		v->e_reg = v->pid_ref_reg - v->pid_fdb_reg;
		v->up_reg = v->Kp_reg * v->e_reg;
		v->uprsat_reg = v->up_reg + v->ui_reg;
		if (v->uprsat_reg > v->pid_out_max)
		  v->pid_out_reg =  v->pid_out_max;
		else if (v->uprsat_reg < v->pid_out_min)
		  v->pid_out_reg =  v->pid_out_min;
		else
		  v->pid_out_reg = v->uprsat_reg;
		v->saterr_reg = v->pid_out_reg - v->uprsat_reg;
		v->ui_reg = v->ui_reg + v->Ki_reg * v->up_reg + v->Kc_reg * v->saterr_reg;
}

// Park变换
void Park(SinglePhaseVector *p)
{
	p->D = p->Alpha * p->CosTheta + p->Beta * p->SinTheta; 
	p->Q = p->Beta * p->CosTheta - p->Alpha * p->SinTheta;
}

// 相位归算(-1，1) => (-pi,pi)
void PERIOD_DEFINE_2PI(float Angle){
	if(Angle > 1.0f)	\
	{													\
		Angle = Angle - 2.0f; \
	}											\
	else if(Angle < -1.0f)					\
	{										\
		Angle = Angle + 2.0f; \
	}											\
}

//锁相环
void SOGI_PLL(SinglePhaseVector *p,SOGI_Filter *v,PIDREG *k)
{
    // 先计算sin和cos值
    p->SinTheta = arm_sin_f32(p->VectorAngle * PI_RAD);
    p->CosTheta = arm_cos_f32(p->VectorAngle * PI_RAD);
    // 将采集电压送入SOGI滤波器
    v->SOGI_Un = p->Value;
    // 计算SOGI状态方程
    SOGI_Filter_Calc(v);
    // 将SOGI的输出更新值Vgird结构体
    p->Alpha = v->SOGI_X2n;
    p->Beta  = v->SOGI_X1n;
    // 派克变换
    Park(p); //p.D = p.Alpha * p.CosTheta + p.Beta * p.SinTheta; p.Q = p.Beta * p.CosTheta - p.Alpha * p.SinTheta;
    // PI计算
    k->pid_fdb_reg = 0;
    k->pid_ref_reg = p->Q;
    pid_reg_calc(k);
    p->VectorAngle = p->VectorAngle + k->pid_out_reg;
    // 归算相位至(-1,1)
    PERIOD_DEFINE_2PI(p->VectorAngle);
}
