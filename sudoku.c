/*======================================================
    > File Name: sudoku.c
    > Author: lyh
    > E-mail:  
    > Other :  
    > Created Time: 2015年04月12日 星期日 20时55分29秒
 =======================================================*/

#include<stdio.h>
#include<stdlib.h>

#define     MAX    81   //格子总数
//状态值
const int status[10] = {0,1,2,4,8,16,32,64,128,256};
                        //000000000~100000000
typedef struct
{
	int  x,y;           //横，纵坐标
	int  select;        //可取值
	int  n;             //可取值个数
	int  value;         //已取值
	char ckey;          //字符标志
}MapType;

typedef struct
{
	MapType  data[MAX];
	int      top;
}Stack;

typedef struct
{
	MapType  data[MAX];
	int      top;
	int      charbase;   
}ReadyQ;                 //候选队

Stack    pass;                         //栈
ReadyQ   ready;                        //候选队
int      Pos[9][9][2] = {};            //数独棋盘
int      C[9]={0},R[9]={0},F[9]={0};   //行，列，宫

//计算该位置取值个数
int Selectnum(int binary)
{
	int    count = 0;
	while(binary)
	{
		count++;
		binary = binary & (binary-1);
	}
	return count;
}

//获取该位置第一个可取值
int Getvalue(int binary)
{
	int i;
	for(i=1;i<=9;i++)
	{
		if(binary & status[i]>0)
			return i;
	}
	return 0;
}

//初始化栈
void Init_stack()
{
	pass.top = -1; 
}

//初始化候选队
void Init_Ready()
{
	ready.top = -1;
	ready.charbase = MAX;
}

//入栈
void PushS(MapType elem)
{
	pass.top++;
	pass.data[pass.top] = elem;
}
//出栈
MapType PopS()
{
	MapType   elem;
	elem = pass.data[pass.top];
	pass.top--;
	return elem;
}

//放入候选队
void PushQ(MapType elem)
{
	int     i;
	ready.top++;
	i = ready.top;
	while(i!=0 && elem.n>ready.data[i-1].n)
	{
		ready.data[i] = ready.data[i-1];
		i--;
	}
	ready.data[i] = elem;
}

int  UpdataStatus(MapType elem,int v)
{
	int         i,x,y;
	
	elem.select = elem.select & (~v);   //与反码取&消去已尝试的数

	C[elem.x] = C[elem.x] | status[v];  //更新行列宫的状态标志
	R[elem.y] = R[elem.y] | status[v];
	F[(elem.x*3+elem.y)/3] = F[(elem.x*3+elem.y)/3] | status[v];

	for(i=ready.top;i>=0;i--)
	{
		x = ready.data[i].x;
		y = ready.data[i].y;
		if(x==elem.x||y==elem.y||((x*3+y)/3)==((elem.x*3+elem.y)/3))  //包含所取数字
		{
		   	ready.data[i].select = (~C[x]) & (~R[y]) & (~F[(x*3+y)/3]) & 511;
			ready.data[i].n = Getvalue(ready.data[i].select);
			if(elem.ckey==ready.data[i].ckey&&elem.ckey>='a'&&(ready.data[i].select&v)==0)
				return 0;
            if(ready.data[i].n==0)
				return 0;         //无解
			for(k=i;k<=ready.top && ready.data[k].n<ready.data[k+1].n;k++)
			{
				t = ready.data[k];
				ready.data[k] = ready.data[k+1];
				ready.data[k+1] = t;
			}
		}
	}
	return 1;
}

//出候选队
int PopQ(MapType *elempoint)
{
	MapType     elem,t;
	int         i,k,x,y,v;
	elem = ready.data[ready.top];
	if(elem.value>0)
	{
		ready.top--;
		*elempoint = elem;
		return 1;
	}
	i = ready.top;
	v = Getvalue(elem.select);
	if(elem.ckey>='a')
	{
		elem.value = v;
		elem.select = elem.select & (~v);
		*elempoint = elem;
		return 1;
	}

	if(UpdataSort(i,v)==0)
		return 0;
	i = --ready.top;
	if(UpdataSort(i,v)==0)
		return 0;
	if(elem.ckey>='a')
	{
		for(i=i-1;i>=0;i--)
		{
			if(ready.data[i].ckey==elem.ckey)
			{
				ready.data[i].value = v;
				if(UpdataSort(i,v)==0)
					return 0;
			}
		}
	}
	*elempoint = elem;
	return 1;
}

//初始化棋盘
int  Init_Map(int OrgMap[9][9])
{
	int  i,j,k;
	for(i=0;i<9;i++)
	{
		for(j=0;j<9;j++)
		{
			k = OrgMap[i][j];
			Pos[i][j][2] = k;
			if(k>='0'&&k<='9')
			{
				Pos[i][j][0] = k-'0';    // (3*i+j)/3 宫
				C[i] = C[i] | status[k-'0'];
				R[j] = R[j] | status[k-'0'];
				F[(3*i+j)/3] = F[(3*i+j)/3] | status[k-'0'];
			}
			else
			{
				Pos[i][j][0] = 0;
			}
		}
	}
	for(i=0;i<9;i++)
	{
		for(j=0;j<9;j++)
		{
			if(Pos[i][j][0]==0)
			{
				Pos[i][j][1] = (~C[i])&(~R[j])&(~F[(i*3+j)/3])&511;
				if(Pos[i][j][1]==0)
					return 0;
			}
		}
	}
	return 1;
}

int main()
{
	FILE*      fp;
	int        i,j;
	int        OrgMap[9][9];
	char       c;

	printf("%d %d %d %d %d\n",C[8],R[8],F[0],F[1],F[2]);
	if((fp=fopen("input.txt","r"))==NULL)
	{
		printf("error!\n");
		exit(0);
	}
	//读取棋盘数据
	for(i=0;i<9;i++)
	{
		for(j=0;j<9;j++)
		{
			if((c=fgetc(fp))!='\n')
			{
				OrgMap[i][j] = ('0'<=c&&c<='9')?(c-'0'):c;
			}
		}
	}
	fclose(fp);
	return 0;
}
