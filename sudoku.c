/*======================================================
    > File Name: sudoku.c
    > Author: lyh
    > E-mail:  
    > Other :  
    > Created Time: 2015年04月12日 星期日 20时55分29秒
 =======================================================*/

#include<stdio.h>
#include<stdlib.h>

#define     MAX          81       //格子总数
#define     KEEPODD      341      //101010101 保留奇数
#define     KEEPEVEN     170      //010101010 保留偶数

int         number=0;

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
int      Pos[9][9][3] = {};            //数独棋盘
int      C[9]={0},R[9]={0},F[9]={0};   //行，列，宫
char     str[10];
//判断宫
int transF(int x,int y)
{   return (x/3)*3+(y/3);  }

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

char*  Inputb(int binary)
{
	int   i;
	for(i=1;i<10;i++)
	{
		if((binary & status[i])>0)
			str[i-1] = '1';
		else
			str[i-1] = '0';
	}
	str[9] = '\0';
	return str;
}

//获取该位置可取值
int Getvalue(int binary,int key)
{
	int i;
	for(i=key;i<=9;i++)
	{
		if((binary & status[i])>0)
			return i;
	}
	return 0;
}

int JudgeCRF(MapType elem,int v)
{
	if(((C[elem.x] | R[elem.y] | F[transF(elem.x,elem.y)]) & status[v])==0&&(elem.select&status[v])==0)
		return 0;
	return 1;
}
//判断是否同列或同行或同宫
int Samekind(MapType elem1,MapType elem2)
{
	if(elem1.x==elem2.x||elem1.y==elem2.y||transF(elem1.x,elem1.y)==transF(elem2.x,elem2.y))
		return 1;
	return 0;
}

//更新同行列宫的状态
int UpdateCRF(MapType elem,int v)
{
	C[elem.x] = C[elem.x] | status[v];  //更新行列宫的状态标志
	R[elem.y] = R[elem.y] | status[v];
	F[transF(elem.x,elem.y)] = F[transF(elem.x,elem.y)] | status[v];
}

//恢复行列宫状态
void RenewCRF(MapType elem,int v)
{
	C[elem.x] = C[elem.x] & (~status[v]);
	R[elem.y] = R[elem.y] & (~status[v]);
	F[transF(elem.x,elem.y)] = F[transF(elem.x,elem.y)] & (~status[v]);
}

//判断和更新
int JudgeUpdate(MapType elem,int v)
{
	int     i,j,err=1;
	UpdateCRF(elem,v);
	if(elem.ckey=='e'||elem.ckey=='o'||elem.ckey=='0')  //数字或奇偶数
	{
		for(i=0;i<=ready.top;i++)
			if(Samekind(elem,ready.data[i])&&(ready.data[i].select & status[v])>0)
			{
				ready.data[i].select = ready.data[i].select & (~status[v]);   //与反码取&消去已尝试的数
				ready.data[i].n--;
				if(ready.data[i].select==0)
				{
					RenewCRF(elem,v);
					for(j=i;j>=0;j--)
						if(Samekind(elem,ready.data[j])&& (JudgeCRF(ready.data[j],v)==0))
						{
							if(ready.data[j].ckey=='e'&&(v%2)!=0)
								continue;
							if(ready.data[j].ckey=='o'&&(v%2)==0)
								continue;
							ready.data[j].select = ready.data[j].select | status[v];
							ready.data[j].n++;
						}
					return 0;
				}
			}
		return 1;
	}
	else           //字母
	{
		for(i=0;i<=ready.top;i++)
			if(elem.ckey==ready.data[i].ckey)
			{
				if((ready.data[i].select & status[v])==0)
				{
					for(j=ready.charbase;j<MAX;j++)
						if(elem.ckey==ready.data[j].ckey)
						{
							RenewCRF(ready.data[j],v);
							ready.data[j].select = ready.data[j].select | status[v];
							ready.data[j].n++;
							ready.data[j].value = 0;
							ready.top++;
							ready.data[ready.top] = ready.data[j];
						    ready.charbase++;	
						}
					RenewCRF(elem,v);
					return 0;
				}
				else
				{
					ready.data[i].select = ready.data[i].select & (~status[v]);
					ready.charbase--;
					ready.data[ready.charbase] = ready.data[i];
					ready.data[ready.charbase].value = v;
					ready.data[ready.charbase].n--;
					ready.data[i] = ready.data[ready.top];     //用队顶填补空缺
					ready.top--;
					UpdateCRF(ready.data[ready.charbase],v);
					Pos[ready.data[ready.charbase].x][ready.data[ready.charbase].y][0] = v;
				}
			}
		for(i=0;i<=ready.top&&err!=0;i++)
		{
			if(Samekind(elem,ready.data[i])&&(ready.data[i].select & status[v])>0)
			{
				ready.data[i].select = ready.data[i].select & (~status[v]);
				ready.data[i].n--;
				if(ready.data[i].select==0)
				{
					RenewCRF(elem,v);
					err = 0;
					break;
				}
				continue;
			}
			for(j=ready.charbase;j<MAX;j++)
			{
				int   k;
				if(elem.ckey==ready.data[j].ckey)
				if(Samekind(ready.data[j],ready.data[i])&&(ready.data[i].select & status[v])>0)
				{
					ready.data[i].select = ready.data[i].select & (~status[v]);
					ready.data[i].n--;
					if(ready.data[i].select==0)
					{
						err = 0;
						for(k=j;k>=ready.charbase;k--)
							if(ready.data[k].ckey==elem.ckey)
								RenewCRF(ready.data[k],v);
						break;
					}
					break;
				}
			}
		}
		if(err==0)
		{
			for(j=i;j>=0;j--)
			{	
				if(JudgeCRF(ready.data[j],v))
					continue;
				if(Samekind(elem,ready.data[j]))
				{
					if(ready.data[j].ckey=='e'&&(v%2)!=0)
						continue;
					if(ready.data[j].ckey=='o'&&(v%2)==0)
						continue;
					ready.data[j].select = ready.data[j].select | status[v];
					ready.data[j].n++;
				}
				for(i=ready.charbase;i<MAX;i++)
				{
					if(elem.ckey==ready.data[i].ckey)
					if(Samekind(ready.data[j],ready.data[i]))
					{
						if(ready.data[j].ckey=='e'&&(v%2)!=0)
							continue;
						if(ready.data[j].ckey=='o'&&(v%2)==0)
							continue;
						ready.data[j].select = ready.data[j].select | status[v];
						ready.data[j].n++;
						break;
					}
				}

			}
			for(j=ready.charbase;j<MAX;j++)
				if(elem.ckey==ready.data[j].ckey)
				{
					ready.data[j].select = ready.data[j].select | status[v];
					ready.data[j].n++;
					ready.data[j].value = 0;
					ready.top++;
					ready.data[ready.top] = ready.data[j];
					ready.charbase++;	
				}
			return 0;

		}
		return 1;
	}
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
    printf("PushS:[%d,%d] select=%s value=%d  n=%d\n",elem.x,elem.y,Inputb(elem.select),elem.value,elem.n);
}
//出栈
MapType PopS()
{
	MapType   elem;
	elem = pass.data[pass.top];
	pass.top--;
	  printf(" PopS:[%d,%d] select=%s value=%d  n=%d\n",elem.x,elem.y,Inputb(elem.select),elem.value,elem.n);
	return elem;
}

//恢复同类的原状态
void RenewKind(MapType elem,int v)
{
	int     i,j,k;
	
	RenewCRF(elem,v);
	if(elem.ckey=='e'||elem.ckey=='o'||elem.ckey=='0')  //数字或奇偶数
	{
		for(i=0;i<=ready.top;i++)
			if(Samekind(elem,ready.data[i])&&(JudgeCRF(ready.data[i],v)==0))
			{
				if(ready.data[i].ckey=='e'&&(v%2)!=0)
					continue;
				if(ready.data[i].ckey=='o'&&(v%2)==0)
					continue;
				ready.data[i].select = ready.data[i].select | status[v];   
				ready.data[i].n++;
			}
	}
	else           //字母
	{
		for(k=ready.charbase;k<MAX;k++)
			if(ready.data[k].ckey==elem.ckey)
				RenewCRF(ready.data[k],v);

		for(i=0;i<=ready.top;i++)
		{
			if(JudgeCRF(ready.data[i],v))
				continue;
			if(Samekind(elem,ready.data[i]))
			{
				if(ready.data[i].ckey=='e'&&(v%2)!=0)
					continue;
				if(ready.data[i].ckey=='o'&&(v%2)==0)
					continue;
				ready.data[i].select = ready.data[i].select | status[v];
				ready.data[i].n++;
				continue;
			}
			for(j=ready.charbase;j<MAX;j++)
			{
				if(ready.data[j].ckey==elem.ckey)
				if(Samekind(ready.data[j],ready.data[i]))
				{
					if(ready.data[i].ckey=='e'&&(v%2)!=0)
						continue;
					if(ready.data[i].ckey=='o'&&(v%2)==0)
						continue;
					ready.data[i].select = ready.data[i].select & (~status[v]);
					ready.data[i].n++;
					break;
				}
			}
		}
		for(j=ready.charbase;j<MAX;j++)
			if(ready.data[j].ckey==elem.ckey)
			{
				ready.top++;
				ready.data[j].select = ready.data[j].select | status[v];
				ready.data[j].n++;
				ready.data[j].value = 0;
				ready.data[ready.top] = ready.data[j];
				ready.charbase++;
			}
	}
}

//放入候选队
void PushQ(MapType elem)
{
	int   v=elem.value;
	RenewKind(elem,v);
	elem.select = elem.select | status[v];
	elem.n++;
	elem.value = 0;
	ready.top++;
	ready.data[ready.top] = elem;
	  printf("PushQ:[%d,%d] select=%s value=%d  n=%d\n",elem.x,elem.y,Inputb(elem.select),elem.value,elem.n);
}

//出候选队
MapType  PopQ()
{
	MapType  elem;
	int         i,k=0;
	for(i=1;i<=ready.top;i++)
	{
		if(ready.data[i].n<ready.data[k].n)
			k = i;
	}
	elem = ready.data[k];
	ready.data[k] = ready.data[ready.top];
	ready.top--;
	elem.value = 0;
	elem.n--;
	  printf(" PopQ:[%d,%d] select=%s value=%d  n=%d\n",elem.x,elem.y,Inputb(elem.select),elem.value,elem.n);
	return elem;
}

//初始化棋盘
int  Init_Map(int OrgMap[9][9])
{
	int  i,j,k;
	for(i=0;i<9;i++)
	{
		printf("\n");
		for(j=0;j<9;j++)
		{
			k = OrgMap[i][j];
			Pos[i][j][2] = k;
			if(k>'0'&&k<='9')
			{
				Pos[i][j][0] = k-'0';    
				C[i] = C[i] | status[k-'0'];
				R[j] = R[j] | status[k-'0'];
				F[transF(i,j)] = F[transF(i,j)] | status[k-'0'];
			}
			else
			{
				Pos[i][j][0] = 0;
			}
			printf("%d ",Pos[i][j][0]);
		}
		printf("\n");
	}
	for(i=0;i<9;i++)
	{
		for(j=0;j<9;j++)
		{
			if(Pos[i][j][0]==0)
			{
				Pos[i][j][1] = (~C[i])&(~R[j])&(~F[transF(i,j)])&511;
				if(Pos[i][j][2]=='o')
					Pos[i][j][1] = Pos[i][j][1] & KEEPODD;     //过滤偶数
				if(Pos[i][j][2]=='e')
					Pos[i][j][1] = Pos[i][j][1] & KEEPEVEN;    //过滤奇数
				if(Pos[i][j][1]==0)
					return 0;
				ready.top++;
				ready.data[ready.top].value = 0;
				ready.data[ready.top].x = i;
				ready.data[ready.top].y = j;
				ready.data[ready.top].select = Pos[i][j][1];
				ready.data[ready.top].n = Selectnum(Pos[i][j][1]);
				ready.data[ready.top].ckey = Pos[i][j][2];

				printf("Pos:[%d,%d] select=%s n=%d  ready:top=%d\n",i,j,Inputb(Pos[i][j][1]),ready.data[ready.top].n,ready.top);
			}
		}
	}
	return 1;
}
//求解
int Solve(int OrgMap[9][9])
{
	int    i;
	MapType     elem;
	Init_stack();
	Init_Ready();
	if(Init_Map(OrgMap)==0)
		return 0;
	else
	{
		while(ready.top!=-1)
		{
			elem = PopQ();
			PushS(elem);
			while(pass.top!=-1)
			{
				int     v,v0;
				elem = pass.data[pass.top];
				while(v=Getvalue(elem.select,elem.value+1))
				{
					int   k;
					v0 = elem.value;
					if(v0!=0)
						RenewKind(elem,elem.value);
					pass.data[pass.top].value = v;
					pass.data[pass.top].select = pass.data[pass.top].select & (~status[v]) | status[v0];
					elem = pass.data[pass.top];
					printf("    :[%d,%d] value=%d  select=%d n=%d\n",elem.x,elem.y,elem.value,elem.select,elem.n);
                    k = JudgeUpdate(elem,v);
					printf("-----------------\n");
					for(i=0;i<=ready.top;i++)
						printf("[%d,%d] value=%d select=%s n=%d num=%d\n",ready.data[i].x,ready.data[i].y,ready.data[i].value,Inputb(ready.data[i].select),ready.data[i].n,i);
				    printf("-----------------\n");
					if(k)
					{
//						printf("    :[%d,%d] value=%d  select=%d n=%d\n",elem.x,elem.y,elem.value,elem.select,elem.n);
						Pos[elem.x][elem.y][0] = v;
						if(ready.top!=-1)
							elem = PopQ();
						else
							return 1;
						PushS(elem);
//						printf("PQPS:[%d,%d] select=%d n=%d num=%d\n",elem.x,elem.y,elem.select,elem.n,number++);
					}
				}
				elem = PopS();
				RenewKind(elem,elem.value);
				PushQ(elem);
//				printf("PSPQ:[%d,%d] select=%d n=%d\n",elem.x,elem.y,elem.select,elem.n);
			}
 /*           for(i=0;i<=ready.top;i++)
				printf("[%d,%d] select=%d value=%d n=%d\n",ready.data[i].x,ready.data[i].y,ready.data[i].select,ready.data[i].value,ready.data[i].n);
*/
				return 0;
		}
		return 1;
	}
}

int main()
{
	FILE*      fp;
	int        i,j;
	int        OrgMap[9][9];
	char       c;
	MapType    elem;
	char       filename[20];

	scanf("%s",filename);

	printf("%d %d %d %d %d\n",C[8],R[8],F[0],F[1],F[2]);
	if((fp=fopen(filename,"r"))==NULL)
	{
		printf("error!\n");
		exit(0);
	}
	//读取棋盘数据
	for(i=0;i<9;i++)
	{
		for(j=0;j<10;j++)
		{
			if((c=fgetc(fp))!='\n')
			{
				OrgMap[i][j] = c;
				printf("%c",OrgMap[i][j]);
			}
		}
		printf("\n");
	}
	fclose(fp);
	if(Solve(OrgMap))
	{
		printf("\n\n");
		for(i=0;i<9;i++)
		{
			for(j=0;j<9;j++)
				printf("%d ",Pos[i][j][0]);
			printf("\n");
		}
	}
	else
		printf("impossible\n");
	return 0;
}
