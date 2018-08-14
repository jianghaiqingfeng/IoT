#include "malloc.h"	   

//内存池(32字节对齐)
__align(32) u8 mem1base[MEM1_MAX_SIZE];													          //内部SRAM内存池
__align(32) u8 mem2base[MEM2_MAX_SIZE] __attribute__((at(0X68000000)));		//外部SRAM内存池
__align(32) u8 mem3base[MEM3_MAX_SIZE] __attribute__((at(0X10000000)));		//内部CCM内存池

//内存管理表
u16 mem1mapbase[MEM1_ALLOC_TABLE_SIZE];													//内部SRAM内存池MAP
u16 mem2mapbase[MEM2_ALLOC_TABLE_SIZE] __attribute__((at(0X68000000+MEM2_MAX_SIZE)));	//外部SRAM内存池MAP
u16 mem3mapbase[MEM3_ALLOC_TABLE_SIZE] __attribute__((at(0X10000000+MEM3_MAX_SIZE)));	//内部CCM内存池MAP

//内存管理参数	   
const u32 memtblsize[SRAMBANK]={MEM1_ALLOC_TABLE_SIZE,MEM2_ALLOC_TABLE_SIZE,MEM3_ALLOC_TABLE_SIZE};	//内存表大小
const u32 memblksize[SRAMBANK]={MEM1_BLOCK_SIZE,MEM2_BLOCK_SIZE,MEM3_BLOCK_SIZE};					//内存分块大小
const u32 memsize[SRAMBANK]={MEM1_MAX_SIZE,MEM2_MAX_SIZE,MEM3_MAX_SIZE};							    //内存总大小


//内存管理控制器
struct _m_mallco_dev mallco_dev=
{
	.init    = my_mem_init,					    	               //内存初始化
	.perused = my_mem_perused,						               //内存使用率
	.membase = {mem1base, mem2base, mem3base},			     //内存池
	.memmap  = {mem1mapbase, mem2mapbase, mem3mapbase},  //内存管理状态表
	.memrdy  = {MEMBSY, MEMBSY, MEMBSY}	    	           //内存管理未就绪
};


/************************************************************************
** 函数名称: mymemcpy									
** 函数功能: 复制内存
** 入口参数: void *des:目的地址
**           void *src:原地址
**           u32 n:  需要复制的内存长度(以字节为单位) 
** 出口参数: 												
************************************************************************/
void mymemcpy(void *des,void *src,u32 n)  
{  
	u8 *xdes=des;
	u8 *xsrc=src; 
	while(n--)
		*xdes++=*xsrc++;  
} 


/************************************************************************
** 函数名称: mymemset									
** 函数功能: 设置内存
** 入口参数: void *s:内存首地址
**           u8 c:要设置的值
**           u32 count:  需要设置的内存长度(以字节为单位) 
** 出口参数: 												
************************************************************************/
void mymemset(void *s,u8 c,u32 count)  
{  
	u8 *xs = s;  
	while(count--)
		*xs++=c;  
}	   


/************************************************************************
** 函数名称: my_mem_init									
** 函数功能: 内存管理初始化  
** 入口参数: u8 memx:所属内存块 
** 出口参数: 												
************************************************************************/
void my_mem_init(u8 memx)  
{  
	//清除内存状态表-每一个内存块对应状态表中的一项
	//因为内存管理状态表示16位的，所以需要乘以2
	mymemset( mallco_dev.memmap[memx], 0, memtblsize[memx]*2); 
	//把所管理的内存池全部清理
	mymemset(mallco_dev.membase[memx], 0, memsize[memx]); 
	//表明对应的内存管理已经就绪
	mallco_dev.memrdy[memx] = MEMRDY;
}  

/************************************************************************
** 函数名称: my_mem_perused									
** 函数功能: 计算内存使用率
** 入口参数: u8 memx:所属内存块 
** 出口参数: 使用率(0~100)												
************************************************************************/
u8 my_mem_perused(u8 memx)  
{  
	u32 used=0;  
	u32 i;  
	
	//逐一对每个表项进行统计
	for(i=0;i<memtblsize[memx];i++)  
	{  
		  //如果表项值不为0就表示对应内存块被使用了
			if(mallco_dev.memmap[memx][i])
				used++; 
	} 
	
	//计算百分比
	return (used*100)/(memtblsize[memx]);  
}

/************************************************************************
** 函数名称: my_mem_malloc									
** 函数功能: 内存分配(内部调用)
** 入口参数: u8 memx:所属内存块 
**           u32 size:要分配的内存大小(字节)
** 出口参数: 0xFFFFFFFF-分配错误 其他-内存偏移地址 											
************************************************************************/
u32 my_mem_malloc(u8 memx,u32 size)  
{  
	signed long offset=0;  
	u32 nmemb;	//需要的内存块数  
	u32 cmemb = 0;//连续空内存块数
	u32 i;  
	
	//未初始化,先执行初始化 
	if(mallco_dev.memrdy[memx] == MEMBSY)
		mallco_dev.init(memx);
	
	//不需要分配
  if(size == 0)
		return 0xFFFFFFFF;
	
	//获取需要分配的连续内存块数
  nmemb = size/memblksize[memx];  
	//超出不满一个内存块的部分也要申请一个内存块
  if(size%memblksize[memx])
		nmemb++;  
	
	//搜索内存块，从后往前搜索
	for(offset=memtblsize[memx]-1;offset>=0;offset--)//搜索整个内存控制区  
	{    
		//如果内存块没有被使用就代表找到一个空块
		if(mallco_dev.memmap[memx][offset] == 0)
		{
			//连续空内存块数增加
			cmemb++;
		}
		else 
		{
			//连续内存块清零--只要找到一个被使用的就表示不连续了，要重新开始找
			cmemb=0;								
		}
		
		//找到了连续nmemb个空内存块
		if(cmemb == nmemb)							
		{
			//标注内存块非空--并且是连续nmemb个非空块
			for(i=0;i<nmemb;i++)  					
			{  
				mallco_dev.memmap[memx][offset+i] = nmemb;  
			}  
			//返回偏移地址 
			return (offset*memblksize[memx]); 
		}
	}  
	//未找到符合分配条件的内存块 
	return 0xFFFFFFFF; 
}  

/************************************************************************
** 函数名称: my_mem_free									
** 函数功能: 释放内存(内部调用) 
** 入口参数: u8 memx:所属内存块
**           u32 offset:内存地址偏移
** 出口参数: 0-释放成功;1-释放失败										
************************************************************************/
u8 my_mem_free(u8 memx,u32 offset)  
{  
	int i; 

  //未初始化,先执行初始化	
	if(mallco_dev.memrdy[memx] == MEMBSY)
	{
		mallco_dev.init(memx);    
    return 1;//未初始化  
   }  
	
	//偏移在内存池内. 
	if(offset < memsize[memx])
	{  
		int index = offset/memblksize[memx];			//偏移所在内存块号码  
		int nmemb = mallco_dev.memmap[memx][index];	//内存块数量
		
		for(i=0;i<nmemb;i++)  						//内存块清零
		{  
			mallco_dev.memmap[memx][index+i] = 0;  
		}  
		return 0;  
	}
	else 
		return 2;//偏移超区了.  
}  

/************************************************************************
** 函数名称: myfree									
** 函数功能: 释放内存(外部调用) 
** 入口参数: u8 memx:所属内存块
**           void *ptr:内存地址偏移
** 出口参数: 无									
************************************************************************/
void myfree(u8 memx,void *ptr)  
{  
	u32 offset; 
	
  //地址为0. 
	if(ptr==NULL)
		return; 
	
	//计算指针在相应内存块中的偏移量
 	offset = (u32)ptr-(u32)mallco_dev.membase[memx]; 

  //释放内存 	
  my_mem_free(memx,offset);	     
}

/************************************************************************
** 函数名称: mymalloc									
** 函数功能: 内存分配(内部调用)
** 入口参数: u8 memx:所属内存块 
**           u32 size:要分配的内存大小(字节)
** 出口参数: 分配到的内存首地址.											
************************************************************************/
void *mymalloc(u8 memx,u32 size)  
{  
	u32 offset;   
	
	offset=my_mem_malloc(memx,size); 
	
	if(offset == 0xFFFFFFFF)
		return NULL;  
	else 
		return (void*)((u32)mallco_dev.membase[memx]+offset);  
}  

/************************************************************************
** 函数名称: myrealloc									
** 函数功能: 重新分配内存(外部调用)
** 入口参数: u8 memx:所属内存块 
**           void *ptr:旧内存首地址
**           u32 size:要分配的内存大小(字节)
** 出口参数: 新分配到的内存首地址.										
************************************************************************/
void *myrealloc(u8 memx,void *ptr,u32 size)  
{  
	u32 offset;    
	
	//先申请一段内存
	offset = my_mem_malloc(memx,size);  

	//申请失败
	if(offset == 0xFFFFFFFF)
		return NULL;     
	else  
	{  		
    //拷贝旧内存内容到新内存  		
		mymemcpy((void*)((u32)mallco_dev.membase[memx]+offset),ptr,size);	 
		
		//释放旧内存
		myfree(memx,ptr);  			
		
		//返回新内存首地址		
		return (void*)((u32)mallco_dev.membase[memx]+offset);  				
	}  
}












