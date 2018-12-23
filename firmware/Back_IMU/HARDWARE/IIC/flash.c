
#include "include.h"
#include "flash.h"
#include "mpu6050.h"
#include "hml5833l.h"
#include "flash_w25.h"
#include "LIS3MDL.h" 
//��ȡָ����ַ�İ���(16λ����) 
//faddr:����ַ 
//����ֵ:��Ӧ����.
u32 STMFLASH_ReadWord(u32 faddr)
{
	return *(vu32*)faddr; 
}  
//��ȡĳ����ַ���ڵ�flash����
//addr:flash��ַ
//����ֵ:0~11,��addr���ڵ�����
uint16_t STMFLASH_GetFlashSector(u32 addr)
{
	if(addr<ADDR_FLASH_SECTOR_1)return FLASH_Sector_0;
	else if(addr<ADDR_FLASH_SECTOR_2)return FLASH_Sector_1;
	else if(addr<ADDR_FLASH_SECTOR_3)return FLASH_Sector_2;
	else if(addr<ADDR_FLASH_SECTOR_4)return FLASH_Sector_3;
	else if(addr<ADDR_FLASH_SECTOR_5)return FLASH_Sector_4;
	else if(addr<ADDR_FLASH_SECTOR_6)return FLASH_Sector_5;
	else if(addr<ADDR_FLASH_SECTOR_7)return FLASH_Sector_6;
	else if(addr<ADDR_FLASH_SECTOR_8)return FLASH_Sector_7;
	else if(addr<ADDR_FLASH_SECTOR_9)return FLASH_Sector_8;
	else if(addr<ADDR_FLASH_SECTOR_10)return FLASH_Sector_9;
	else if(addr<ADDR_FLASH_SECTOR_11)return FLASH_Sector_10; 
	return FLASH_Sector_11;	
}
//��ָ����ַ��ʼд��ָ�����ȵ�����
//�ر�ע��:��ΪSTM32F4������ʵ��̫��,û�취���ر�����������,���Ա�����
//         д��ַ�����0XFF,��ô���Ȳ������������Ҳ�������������.����
//         д��0XFF�ĵ�ַ,�����������������ݶ�ʧ.����д֮ǰȷ��������
//         û����Ҫ����,��������������Ȳ�����,Ȼ����������д. 
//�ú�����OTP����Ҳ��Ч!��������дOTP��!
//OTP�����ַ��Χ:0X1FFF7800~0X1FFF7A0F
//WriteAddr:��ʼ��ַ(�˵�ַ����Ϊ4�ı���!!)
//pBuffer:����ָ��
//NumToWrite:��(32λ)��(����Ҫд���32λ���ݵĸ���.) 
void STMFLASH_Write(u32 WriteAddr,u32 *pBuffer,u32 NumToWrite)	
{ 
  FLASH_Status status = FLASH_COMPLETE;
	u32 addrx=0;
	u32 endaddr=0;	
  if(WriteAddr<STM32_FLASH_BASE||WriteAddr%4)return;	//�Ƿ���ַ
	FLASH_Unlock();									//���� 
  FLASH_DataCacheCmd(DISABLE);//FLASH�����ڼ�,�����ֹ���ݻ���
 		
	addrx=WriteAddr;				//д�����ʼ��ַ
	endaddr=WriteAddr+NumToWrite*4;	//д��Ľ�����ַ
	if(addrx<0X1FFF0000)			//ֻ�����洢��,����Ҫִ�в�������!!
	{
		while(addrx<endaddr)		//ɨ��һ���ϰ�.(�Է�FFFFFFFF�ĵط�,�Ȳ���)
		{
			if(STMFLASH_ReadWord(addrx)!=0XFFFFFFFF)//�з�0XFFFFFFFF�ĵط�,Ҫ�����������
			{   
				status=FLASH_EraseSector(STMFLASH_GetFlashSector(addrx),VoltageRange_3);//VCC=2.7~3.6V֮��!!
				if(status!=FLASH_COMPLETE)break;	//����������
			}else addrx+=4;
		} 
	}
	if(status==FLASH_COMPLETE)
	{
		while(WriteAddr<endaddr)//д����
		{
			if(FLASH_ProgramWord(WriteAddr,*pBuffer)!=FLASH_COMPLETE)//д������
			{ 
				break;	//д���쳣
			}
			WriteAddr+=4;
			pBuffer++;
		} 
	}
  FLASH_DataCacheCmd(ENABLE);	//FLASH��������,�������ݻ���
	FLASH_Lock();//����
} 

//��ָ����ַ��ʼ����ָ�����ȵ�����
//ReadAddr:��ʼ��ַ
//pBuffer:����ָ��
//NumToRead:��(4λ)��
void STMFLASH_Read(u32 ReadAddr,u32 *pBuffer,u32 NumToRead)   	
{
	u32 i;
	for(i=0;i<NumToRead;i++)
	{
		pBuffer[i]=STMFLASH_ReadWord(ReadAddr);//��ȡ4���ֽ�.
		ReadAddr+=4;//ƫ��4���ֽ�.	
	}
}
#define SIZE_PARAM 100
u8 FLASH_READ_BUF[SIZE_PARAM]={0};
u8 FLASH_Buffer[SIZE_PARAM]={0};
u32 FLASH_SIZE=16*1024*1024;	//FLASH ��СΪ16�ֽ�
void READ_PARM(void)
{
u16 cnt=1;
//STMFLASH_Read(FLASH_SAVE_ADDR,(u32*)FLASH_READ_BUF,SIZE);	
W25QXX_Read(FLASH_READ_BUF,FLASH_SIZE-100,SIZE_PARAM);					//�ӵ�����100����ַ����ʼ,����SIZE���ֽ�
mpu6050.Gyro_Offset.x=(vs16)(FLASH_READ_BUF[1]<<8|FLASH_READ_BUF[0]);
mpu6050.Gyro_Offset.y=(vs16)(FLASH_READ_BUF[3]<<8|FLASH_READ_BUF[2]);
mpu6050.Gyro_Offset.z=(vs16)(FLASH_READ_BUF[5]<<8|FLASH_READ_BUF[4]);
	
mpu6050.Acc_Offset.x=(vs16)(FLASH_READ_BUF[7]<<8|FLASH_READ_BUF[6]);
mpu6050.Acc_Offset.y=(vs16)(FLASH_READ_BUF[9]<<8|FLASH_READ_BUF[8]);
mpu6050.Acc_Offset.z=(vs16)(FLASH_READ_BUF[11]<<8|FLASH_READ_BUF[10]);
	
ak8975.Mag_Offset.x=(vs16)(FLASH_READ_BUF[13]<<8|FLASH_READ_BUF[12]);
ak8975.Mag_Offset.y=(vs16)(FLASH_READ_BUF[15]<<8|FLASH_READ_BUF[14]);
ak8975.Mag_Offset.z=(vs16)(FLASH_READ_BUF[17]<<8|FLASH_READ_BUF[16]);
	
ak8975.Mag_Gain.x =(float)((vs16)((FLASH_READ_BUF[19]<<8|FLASH_READ_BUF[18])))/100.;
ak8975.Mag_Gain.y=(float)((vs16)((FLASH_READ_BUF[21]<<8|FLASH_READ_BUF[20])))/100.;
ak8975.Mag_Gain.z =(float)((vs16)((FLASH_READ_BUF[23]<<8|FLASH_READ_BUF[22])))/100.;
	
	
lis3mdl.Gyro_Offset.x=(vs16)(FLASH_READ_BUF[25]<<8|FLASH_READ_BUF[24]);
lis3mdl.Gyro_Offset.y=(vs16)(FLASH_READ_BUF[27]<<8|FLASH_READ_BUF[26]);
lis3mdl.Gyro_Offset.z=(vs16)(FLASH_READ_BUF[29]<<8|FLASH_READ_BUF[28]);
	
lis3mdl.Acc_Offset.x=(vs16)(FLASH_READ_BUF[31]<<8|FLASH_READ_BUF[30]);
lis3mdl.Acc_Offset.y=(vs16)(FLASH_READ_BUF[33]<<8|FLASH_READ_BUF[32]);
lis3mdl.Acc_Offset.z=(vs16)(FLASH_READ_BUF[35]<<8|FLASH_READ_BUF[34]);
	
lis3mdl.Mag_Offset.x=(vs16)(FLASH_READ_BUF[37]<<8|FLASH_READ_BUF[36]);
lis3mdl.Mag_Offset.y=(vs16)(FLASH_READ_BUF[39]<<8|FLASH_READ_BUF[38]);
lis3mdl.Mag_Offset.z=(vs16)(FLASH_READ_BUF[41]<<8|FLASH_READ_BUF[40]);
	
lis3mdl.Mag_Gain.x =(float)((vs16)((FLASH_READ_BUF[43]<<8|FLASH_READ_BUF[42])))/100.;
lis3mdl.Mag_Gain.y=(float)((vs16)((FLASH_READ_BUF[45]<<8|FLASH_READ_BUF[44])))/100.;
lis3mdl.Mag_Gain.z =(float)((vs16)((FLASH_READ_BUF[47]<<8|FLASH_READ_BUF[46])))/100.;	

lis3mdl.Off_3d.x=(vs16)(FLASH_READ_BUF[49]<<8|FLASH_READ_BUF[48]);
lis3mdl.Off_3d.y=(vs16)(FLASH_READ_BUF[51]<<8|FLASH_READ_BUF[50]);
lis3mdl.Off_3d.z=(vs16)(FLASH_READ_BUF[53]<<8|FLASH_READ_BUF[52]);
	
lis3mdl.Gain_3d.x =(float)((vs16)((FLASH_READ_BUF[55]<<8|FLASH_READ_BUF[54])))/1000.;
lis3mdl.Gain_3d.y=(float)((vs16)((FLASH_READ_BUF[57]<<8|FLASH_READ_BUF[56])))/1000.;
lis3mdl.Gain_3d.z =(float)((vs16)((FLASH_READ_BUF[59]<<8|FLASH_READ_BUF[58])))/1000.;

lis3mdl.Mag_Offseto.x=(vs16)(FLASH_READ_BUF[61]<<8|FLASH_READ_BUF[60]);
lis3mdl.Mag_Offseto.y=(vs16)(FLASH_READ_BUF[63]<<8|FLASH_READ_BUF[62]);
lis3mdl.Mag_Offseto.z=(vs16)(FLASH_READ_BUF[65]<<8|FLASH_READ_BUF[64]);
	
lis3mdl.Mag_Gaino.x =(float)((vs16)((FLASH_READ_BUF[67]<<8|FLASH_READ_BUF[66])))/100.;
lis3mdl.Mag_Gaino.y=(float)((vs16)((FLASH_READ_BUF[69]<<8|FLASH_READ_BUF[68])))/100.;
lis3mdl.Mag_Gaino.z =(float)((vs16)((FLASH_READ_BUF[71]<<8|FLASH_READ_BUF[70])))/100.;	

if(fabs(lis3mdl.Off_3d.x)<400&&fabs(lis3mdl.Off_3d.y)<400&&fabs(lis3mdl.Off_3d.z)<400)
	module.acc=2;
else
	module.acc=1;

if(fabs(lis3mdl.Gyro_Offset.x)<200&&fabs(lis3mdl.Gyro_Offset.y)<200&&fabs(lis3mdl.Gyro_Offset.z)<200)
	module.gyro=2;
else
	module.gyro=1;

if((fabs(lis3mdl.Off_3d.x)>10||fabs(lis3mdl.Off_3d.y)>10||fabs(lis3mdl.Off_3d.z)>10)
	&&(fabs(lis3mdl.Off_3d.x)<600&&fabs(lis3mdl.Off_3d.y)<600&&fabs(lis3mdl.Off_3d.z)<600))
lis3mdl.Cali_3d=1;
}

void WRITE_PARM(void)
{ 

int16_t _temp;
u8 cnt=0;

_temp=(int16_t)mpu6050.Gyro_Offset.x;
FLASH_Buffer[cnt++]=BYTE0(_temp);
FLASH_Buffer[cnt++]=BYTE1(_temp);
_temp=(int16_t)mpu6050.Gyro_Offset.y;
FLASH_Buffer[cnt++]=BYTE0(_temp);
FLASH_Buffer[cnt++]=BYTE1(_temp);
_temp=(int16_t)mpu6050.Gyro_Offset.z;
FLASH_Buffer[cnt++]=BYTE0(_temp);
FLASH_Buffer[cnt++]=BYTE1(_temp);

_temp=(int16_t)mpu6050.Acc_Offset.x;
FLASH_Buffer[cnt++]=BYTE0(_temp);
FLASH_Buffer[cnt++]=BYTE1(_temp);
_temp=(int16_t)mpu6050.Acc_Offset.y;
FLASH_Buffer[cnt++]=BYTE0(_temp);
FLASH_Buffer[cnt++]=BYTE1(_temp);
_temp=(int16_t)mpu6050.Acc_Offset.z;
FLASH_Buffer[cnt++]=BYTE0(_temp);
FLASH_Buffer[cnt++]=BYTE1(_temp);
_temp=(int16_t)ak8975.Mag_Offset.x;
FLASH_Buffer[cnt++]=BYTE0(_temp);
FLASH_Buffer[cnt++]=BYTE1(_temp);
_temp=(int16_t)ak8975.Mag_Offset.y;
FLASH_Buffer[cnt++]=BYTE0(_temp);
FLASH_Buffer[cnt++]=BYTE1(_temp);
_temp=(int16_t)ak8975.Mag_Offset.z;
FLASH_Buffer[cnt++]=BYTE0(_temp);
FLASH_Buffer[cnt++]=BYTE1(_temp);


_temp=(int16_t)(ak8975.Mag_Gain.x*100);
FLASH_Buffer[cnt++]=BYTE0(_temp);
FLASH_Buffer[cnt++]=BYTE1(_temp);
_temp=(int16_t)(ak8975.Mag_Gain.y*100);
FLASH_Buffer[cnt++]=BYTE0(_temp);
FLASH_Buffer[cnt++]=BYTE1(_temp);
_temp=(int16_t)(ak8975.Mag_Gain.z*100);
FLASH_Buffer[cnt++]=BYTE0(_temp);
FLASH_Buffer[cnt++]=BYTE1(_temp);


//------lis3mdl
_temp=(int16_t)lis3mdl.Gyro_Offset.x;
FLASH_Buffer[cnt++]=BYTE0(_temp);
FLASH_Buffer[cnt++]=BYTE1(_temp);
_temp=(int16_t)lis3mdl.Gyro_Offset.y;
FLASH_Buffer[cnt++]=BYTE0(_temp);
FLASH_Buffer[cnt++]=BYTE1(_temp);
_temp=(int16_t)lis3mdl.Gyro_Offset.z;
FLASH_Buffer[cnt++]=BYTE0(_temp);
FLASH_Buffer[cnt++]=BYTE1(_temp);

_temp=(int16_t)lis3mdl.Acc_Offset.x;
FLASH_Buffer[cnt++]=BYTE0(_temp);
FLASH_Buffer[cnt++]=BYTE1(_temp);
_temp=(int16_t)lis3mdl.Acc_Offset.y;
FLASH_Buffer[cnt++]=BYTE0(_temp);
FLASH_Buffer[cnt++]=BYTE1(_temp);
_temp=(int16_t)lis3mdl.Acc_Offset.z;
FLASH_Buffer[cnt++]=BYTE0(_temp);
FLASH_Buffer[cnt++]=BYTE1(_temp);
_temp=(int16_t)lis3mdl.Mag_Offset.x;
FLASH_Buffer[cnt++]=BYTE0(_temp);
FLASH_Buffer[cnt++]=BYTE1(_temp);
_temp=(int16_t)lis3mdl.Mag_Offset.y;
FLASH_Buffer[cnt++]=BYTE0(_temp);
FLASH_Buffer[cnt++]=BYTE1(_temp);
_temp=(int16_t)lis3mdl.Mag_Offset.z;
FLASH_Buffer[cnt++]=BYTE0(_temp);
FLASH_Buffer[cnt++]=BYTE1(_temp);


_temp=(int16_t)(lis3mdl.Mag_Gain.x*100);
FLASH_Buffer[cnt++]=BYTE0(_temp);
FLASH_Buffer[cnt++]=BYTE1(_temp);
_temp=(int16_t)(lis3mdl.Mag_Gain.y*100);
FLASH_Buffer[cnt++]=BYTE0(_temp);
FLASH_Buffer[cnt++]=BYTE1(_temp);
_temp=(int16_t)(lis3mdl.Mag_Gain.z*100);
FLASH_Buffer[cnt++]=BYTE0(_temp);
FLASH_Buffer[cnt++]=BYTE1(_temp);



_temp=(int16_t)lis3mdl.Off_3d .x;
FLASH_Buffer[cnt++]=BYTE0(_temp);
FLASH_Buffer[cnt++]=BYTE1(_temp);
_temp=(int16_t)lis3mdl.Off_3d.y;
FLASH_Buffer[cnt++]=BYTE0(_temp);
FLASH_Buffer[cnt++]=BYTE1(_temp);
_temp=(int16_t)lis3mdl.Off_3d.z;
FLASH_Buffer[cnt++]=BYTE0(_temp);
FLASH_Buffer[cnt++]=BYTE1(_temp);

_temp=(int16_t)(lis3mdl.Gain_3d.x*1000);
FLASH_Buffer[cnt++]=BYTE0(_temp);
FLASH_Buffer[cnt++]=BYTE1(_temp);
_temp=(int16_t)(lis3mdl.Gain_3d.y*1000);
FLASH_Buffer[cnt++]=BYTE0(_temp);
FLASH_Buffer[cnt++]=BYTE1(_temp);
_temp=(int16_t)(lis3mdl.Gain_3d.z*1000);
FLASH_Buffer[cnt++]=BYTE0(_temp);
FLASH_Buffer[cnt++]=BYTE1(_temp);

_temp=(int16_t)lis3mdl.Mag_Offseto.x;
FLASH_Buffer[cnt++]=BYTE0(_temp);
FLASH_Buffer[cnt++]=BYTE1(_temp);
_temp=(int16_t)lis3mdl.Mag_Offseto.y;
FLASH_Buffer[cnt++]=BYTE0(_temp);
FLASH_Buffer[cnt++]=BYTE1(_temp);
_temp=(int16_t)lis3mdl.Mag_Offseto.z;
FLASH_Buffer[cnt++]=BYTE0(_temp);
FLASH_Buffer[cnt++]=BYTE1(_temp);


_temp=(int16_t)(lis3mdl.Mag_Gaino.x*100);
FLASH_Buffer[cnt++]=BYTE0(_temp);
FLASH_Buffer[cnt++]=BYTE1(_temp);
_temp=(int16_t)(lis3mdl.Mag_Gaino.y*100);
FLASH_Buffer[cnt++]=BYTE0(_temp);
FLASH_Buffer[cnt++]=BYTE1(_temp);
_temp=(int16_t)(lis3mdl.Mag_Gaino.z*100);
FLASH_Buffer[cnt++]=BYTE0(_temp);
FLASH_Buffer[cnt++]=BYTE1(_temp);

//_temp=(int16_t)(dj_angle_offset[0]*100);
//FLASH_Buffer[cnt++]=BYTE0(_temp);
//FLASH_Buffer[cnt++]=BYTE1(_temp);
//_temp=(int16_t)(dj_angle_offset[1]*100);
//FLASH_Buffer[cnt++]=BYTE0(_temp);
//FLASH_Buffer[cnt++]=BYTE1(_temp);
//_temp=(int16_t)(dj_angle_offset[2]*100);
//FLASH_Buffer[cnt++]=BYTE0(_temp);
//FLASH_Buffer[cnt++]=BYTE1(_temp);

//STMFLASH_Write(FLASH_SAVE_ADDR,(u32*)FLASH_Buffer,SIZE);
	W25QXX_Write((u8*)FLASH_Buffer,FLASH_SIZE-100,SIZE_PARAM);		//�ӵ�����100����ַ����ʼ,д��SIZE���ȵ�����
}









