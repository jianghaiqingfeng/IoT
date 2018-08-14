#include "malloc.h"	   

//�ڴ��(32�ֽڶ���)
__align(32) u8 mem1base[MEM1_MAX_SIZE];													          //�ڲ�SRAM�ڴ��
__align(32) u8 mem2base[MEM2_MAX_SIZE] __attribute__((at(0X68000000)));		//�ⲿSRAM�ڴ��
__align(32) u8 mem3base[MEM3_MAX_SIZE] __attribute__((at(0X10000000)));		//�ڲ�CCM�ڴ��

//�ڴ�����
u16 mem1mapbase[MEM1_ALLOC_TABLE_SIZE];													//�ڲ�SRAM�ڴ��MAP
u16 mem2mapbase[MEM2_ALLOC_TABLE_SIZE] __attribute__((at(0X68000000+MEM2_MAX_SIZE)));	//�ⲿSRAM�ڴ��MAP
u16 mem3mapbase[MEM3_ALLOC_TABLE_SIZE] __attribute__((at(0X10000000+MEM3_MAX_SIZE)));	//�ڲ�CCM�ڴ��MAP

//�ڴ�������	   
const u32 memtblsize[SRAMBANK]={MEM1_ALLOC_TABLE_SIZE,MEM2_ALLOC_TABLE_SIZE,MEM3_ALLOC_TABLE_SIZE};	//�ڴ���С
const u32 memblksize[SRAMBANK]={MEM1_BLOCK_SIZE,MEM2_BLOCK_SIZE,MEM3_BLOCK_SIZE};					//�ڴ�ֿ��С
const u32 memsize[SRAMBANK]={MEM1_MAX_SIZE,MEM2_MAX_SIZE,MEM3_MAX_SIZE};							    //�ڴ��ܴ�С


//�ڴ���������
struct _m_mallco_dev mallco_dev=
{
	.init    = my_mem_init,					    	               //�ڴ��ʼ��
	.perused = my_mem_perused,						               //�ڴ�ʹ����
	.membase = {mem1base, mem2base, mem3base},			     //�ڴ��
	.memmap  = {mem1mapbase, mem2mapbase, mem3mapbase},  //�ڴ����״̬��
	.memrdy  = {MEMBSY, MEMBSY, MEMBSY}	    	           //�ڴ����δ����
};


/************************************************************************
** ��������: mymemcpy									
** ��������: �����ڴ�
** ��ڲ���: void *des:Ŀ�ĵ�ַ
**           void *src:ԭ��ַ
**           u32 n:  ��Ҫ���Ƶ��ڴ泤��(���ֽ�Ϊ��λ) 
** ���ڲ���: 												
************************************************************************/
void mymemcpy(void *des,void *src,u32 n)  
{  
	u8 *xdes=des;
	u8 *xsrc=src; 
	while(n--)
		*xdes++=*xsrc++;  
} 


/************************************************************************
** ��������: mymemset									
** ��������: �����ڴ�
** ��ڲ���: void *s:�ڴ��׵�ַ
**           u8 c:Ҫ���õ�ֵ
**           u32 count:  ��Ҫ���õ��ڴ泤��(���ֽ�Ϊ��λ) 
** ���ڲ���: 												
************************************************************************/
void mymemset(void *s,u8 c,u32 count)  
{  
	u8 *xs = s;  
	while(count--)
		*xs++=c;  
}	   


/************************************************************************
** ��������: my_mem_init									
** ��������: �ڴ�����ʼ��  
** ��ڲ���: u8 memx:�����ڴ�� 
** ���ڲ���: 												
************************************************************************/
void my_mem_init(u8 memx)  
{  
	//����ڴ�״̬��-ÿһ���ڴ���Ӧ״̬���е�һ��
	//��Ϊ�ڴ����״̬��ʾ16λ�ģ�������Ҫ����2
	mymemset( mallco_dev.memmap[memx], 0, memtblsize[memx]*2); 
	//����������ڴ��ȫ������
	mymemset(mallco_dev.membase[memx], 0, memsize[memx]); 
	//������Ӧ���ڴ�����Ѿ�����
	mallco_dev.memrdy[memx] = MEMRDY;
}  

/************************************************************************
** ��������: my_mem_perused									
** ��������: �����ڴ�ʹ����
** ��ڲ���: u8 memx:�����ڴ�� 
** ���ڲ���: ʹ����(0~100)												
************************************************************************/
u8 my_mem_perused(u8 memx)  
{  
	u32 used=0;  
	u32 i;  
	
	//��һ��ÿ���������ͳ��
	for(i=0;i<memtblsize[memx];i++)  
	{  
		  //�������ֵ��Ϊ0�ͱ�ʾ��Ӧ�ڴ�鱻ʹ����
			if(mallco_dev.memmap[memx][i])
				used++; 
	} 
	
	//����ٷֱ�
	return (used*100)/(memtblsize[memx]);  
}

/************************************************************************
** ��������: my_mem_malloc									
** ��������: �ڴ����(�ڲ�����)
** ��ڲ���: u8 memx:�����ڴ�� 
**           u32 size:Ҫ������ڴ��С(�ֽ�)
** ���ڲ���: 0xFFFFFFFF-������� ����-�ڴ�ƫ�Ƶ�ַ 											
************************************************************************/
u32 my_mem_malloc(u8 memx,u32 size)  
{  
	signed long offset=0;  
	u32 nmemb;	//��Ҫ���ڴ����  
	u32 cmemb = 0;//�������ڴ����
	u32 i;  
	
	//δ��ʼ��,��ִ�г�ʼ�� 
	if(mallco_dev.memrdy[memx] == MEMBSY)
		mallco_dev.init(memx);
	
	//����Ҫ����
  if(size == 0)
		return 0xFFFFFFFF;
	
	//��ȡ��Ҫ����������ڴ����
  nmemb = size/memblksize[memx];  
	//��������һ���ڴ��Ĳ���ҲҪ����һ���ڴ��
  if(size%memblksize[memx])
		nmemb++;  
	
	//�����ڴ�飬�Ӻ���ǰ����
	for(offset=memtblsize[memx]-1;offset>=0;offset--)//���������ڴ������  
	{    
		//����ڴ��û�б�ʹ�þʹ����ҵ�һ���տ�
		if(mallco_dev.memmap[memx][offset] == 0)
		{
			//�������ڴ��������
			cmemb++;
		}
		else 
		{
			//�����ڴ������--ֻҪ�ҵ�һ����ʹ�õľͱ�ʾ�������ˣ�Ҫ���¿�ʼ��
			cmemb=0;								
		}
		
		//�ҵ�������nmemb�����ڴ��
		if(cmemb == nmemb)							
		{
			//��ע�ڴ��ǿ�--����������nmemb���ǿտ�
			for(i=0;i<nmemb;i++)  					
			{  
				mallco_dev.memmap[memx][offset+i] = nmemb;  
			}  
			//����ƫ�Ƶ�ַ 
			return (offset*memblksize[memx]); 
		}
	}  
	//δ�ҵ����Ϸ����������ڴ�� 
	return 0xFFFFFFFF; 
}  

/************************************************************************
** ��������: my_mem_free									
** ��������: �ͷ��ڴ�(�ڲ�����) 
** ��ڲ���: u8 memx:�����ڴ��
**           u32 offset:�ڴ��ַƫ��
** ���ڲ���: 0-�ͷųɹ�;1-�ͷ�ʧ��										
************************************************************************/
u8 my_mem_free(u8 memx,u32 offset)  
{  
	int i; 

  //δ��ʼ��,��ִ�г�ʼ��	
	if(mallco_dev.memrdy[memx] == MEMBSY)
	{
		mallco_dev.init(memx);    
    return 1;//δ��ʼ��  
   }  
	
	//ƫ�����ڴ����. 
	if(offset < memsize[memx])
	{  
		int index = offset/memblksize[memx];			//ƫ�������ڴ�����  
		int nmemb = mallco_dev.memmap[memx][index];	//�ڴ������
		
		for(i=0;i<nmemb;i++)  						//�ڴ������
		{  
			mallco_dev.memmap[memx][index+i] = 0;  
		}  
		return 0;  
	}
	else 
		return 2;//ƫ�Ƴ�����.  
}  

/************************************************************************
** ��������: myfree									
** ��������: �ͷ��ڴ�(�ⲿ����) 
** ��ڲ���: u8 memx:�����ڴ��
**           void *ptr:�ڴ��ַƫ��
** ���ڲ���: ��									
************************************************************************/
void myfree(u8 memx,void *ptr)  
{  
	u32 offset; 
	
  //��ַΪ0. 
	if(ptr==NULL)
		return; 
	
	//����ָ������Ӧ�ڴ���е�ƫ����
 	offset = (u32)ptr-(u32)mallco_dev.membase[memx]; 

  //�ͷ��ڴ� 	
  my_mem_free(memx,offset);	     
}

/************************************************************************
** ��������: mymalloc									
** ��������: �ڴ����(�ڲ�����)
** ��ڲ���: u8 memx:�����ڴ�� 
**           u32 size:Ҫ������ڴ��С(�ֽ�)
** ���ڲ���: ���䵽���ڴ��׵�ַ.											
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
** ��������: myrealloc									
** ��������: ���·����ڴ�(�ⲿ����)
** ��ڲ���: u8 memx:�����ڴ�� 
**           void *ptr:���ڴ��׵�ַ
**           u32 size:Ҫ������ڴ��С(�ֽ�)
** ���ڲ���: �·��䵽���ڴ��׵�ַ.										
************************************************************************/
void *myrealloc(u8 memx,void *ptr,u32 size)  
{  
	u32 offset;    
	
	//������һ���ڴ�
	offset = my_mem_malloc(memx,size);  

	//����ʧ��
	if(offset == 0xFFFFFFFF)
		return NULL;     
	else  
	{  		
    //�������ڴ����ݵ����ڴ�  		
		mymemcpy((void*)((u32)mallco_dev.membase[memx]+offset),ptr,size);	 
		
		//�ͷž��ڴ�
		myfree(memx,ptr);  			
		
		//�������ڴ��׵�ַ		
		return (void*)((u32)mallco_dev.membase[memx]+offset);  				
	}  
}












