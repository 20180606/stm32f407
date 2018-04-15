/**
 * @file            mcu_i2s.c
 * @brief           CPU Ƭ���豸I2S����
 * @author          wujique
 * @date            2017��11��16�� ������
 * @version         ����
 * @par             ��Ȩ���� (C), 2013-2023
 * @par History:
 * 1.��    ��:        2017��11��16�� ������
 *   ��    ��:         wujique
 *   �޸�����:   �����ļ�
       	1 Դ����ݼ�ȸ���������С�
		2 �������ڵ�������ҵ��;�����׿��������۳��⣩��������Ȩ��
		3 �ݼ�ȸ�����Ҳ��Դ��빦�����κα�֤����ʹ�������в��ԣ�����Ը���
		4 �������޸�Դ�벢�ַ���������ֱ�����۱���������������뱣��WUJIQUE��Ȩ˵����
		5 �緢��BUG�����Ż�����ӭ�������¡�����ϵ��code@wujique.com
		6 ʹ�ñ�Դ�����൱����ͬ����Ȩ˵����
		7 ���ַ����Ȩ��������ϵ��code@wujique.com
		8 һ�н���Ȩ���ݼ�ȸ���������С�
*/

#include "stm32f4xx.h"
#include "wujique_log.h"

extern s32 fun_sound_set_free_buf(u8 index);
/*

	Ӳ����ʹ��I2S2

*/

/**
 *@brief:      mcu_i2s_init
 *@details:    ��ʼ��I2S�ӿ�Ӳ��
 *@param[in]   void  
 *@param[out]  ��
 *@retval:     
 */
void mcu_i2s_init (void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/*
		LRC 	PB12
		BCLK	PB13
		ADCDAT 	PC2
		DACDAT	PC3
		MCLK	PC6
	*/
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOC, ENABLE);	// ��ʼ��ʱ��

	GPIO_PinAFConfig(GPIOB,	GPIO_PinSource12,		GPIO_AF_SPI2);
	GPIO_PinAFConfig(GPIOB,	GPIO_PinSource13,		GPIO_AF_SPI2);	
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource2,		GPIO_AF6_SPI2);
	GPIO_PinAFConfig(GPIOC,	GPIO_PinSource3,		GPIO_AF_SPI2);	
	GPIO_PinAFConfig(GPIOC,	GPIO_PinSource6,		GPIO_AF_SPI2);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;			// ����ģʽ
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	// �ٶȵȼ�
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		// �������
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	//	��������
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12|GPIO_Pin_13;	
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_6;		
	GPIO_Init(GPIOC,&GPIO_InitStructure);
	
}
/**
 *@brief:      mcu_i2s_config
 *@details:    I2S����
 *@param[in]   u32 AudioFreq   Ƶ��
               u16 Standard    ��׼
               u16 DataFormat  ��ʽ
 *@param[out]  ��
 *@retval:     
 */
void mcu_i2s_config(u32 AudioFreq, u16 Standard,u16 DataFormat)
{
	I2S_InitTypeDef I2S_InitStructure;

	RCC_I2SCLKConfig(RCC_I2S2CLKSource_PLLI2S);				// ����IIS PLLʱ��
	RCC_PLLI2SCmd(ENABLE);											// ʹ��PLL
	while( RCC_GetFlagStatus(RCC_FLAG_PLLI2SRDY) == 0 );	// �ȴ��������
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);					// ��ʼ��IISʱ��

	SPI_I2S_DeInit(SPI2);
	
	I2S_InitStructure.I2S_AudioFreq 	= AudioFreq;			// ������Ƶ����Ƶ�� 
	I2S_InitStructure.I2S_Standard 		= Standard;	//	I2S Philips ��׼
	I2S_InitStructure.I2S_DataFormat 	= DataFormat;		// ���ݳ���16λ
	I2S_InitStructure.I2S_CPOL 			= I2S_CPOL_Low;				// ����״̬��ƽλ��
	I2S_InitStructure.I2S_Mode 			= I2S_Mode_MasterTx;			// ��������
	I2S_InitStructure.I2S_MCLKOutput 	= I2S_MCLKOutput_Enable;	// ��ʱ�����
	I2S_Init(SPI2, &I2S_InitStructure);
	
	I2S_Cmd(SPI2, ENABLE);	// ʹ��IIS
}
/**
 *@brief:      mcu_i2s_dam_init
 *@details:    ��ʼ��I2Sʹ�õ�DMAͨ����˫����ģʽ
 *@param[in]   u16 *buffer0  
               u16 *buffer1  
               u32 len       
 *@param[out]  ��
 *@retval:     
 */
void mcu_i2s_dma_init(u16 *buffer0,u16 *buffer1,u32 len)
{  
	NVIC_InitTypeDef   NVIC_InitStructure;
	DMA_InitTypeDef  DMA_str;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1,ENABLE);	// ʹIIS DMAʱ�� 
	DMA_DeInit(DMA1_Stream4);	//	�ָ���ʼDMA����

	DMA_str.DMA_Channel 				= DMA_Channel_0;  					//	IIS DMAͨ�� 
	DMA_str.DMA_PeripheralBaseAddr 	= (u32)&SPI2->DR;							//	�����ַ
	DMA_str.DMA_Memory0BaseAddr 		= (u32)buffer0;							//	������0
	DMA_str.DMA_DIR 					= DMA_DIR_MemoryToPeripheral;			//	�洢��������ģʽ
	DMA_str.DMA_BufferSize 			= len;										//	���ݳ��� 
	DMA_str.DMA_PeripheralInc 		= DMA_PeripheralInc_Disable;			//	���������ģʽ
	DMA_str.DMA_MemoryInc 			= DMA_MemoryInc_Enable;					//	�洢������ģʽ
	DMA_str.DMA_PeripheralDataSize 	= DMA_PeripheralDataSize_HalfWord;	//	�������ݳ���16λ
	DMA_str.DMA_MemoryDataSize 		= DMA_MemoryDataSize_HalfWord;		//	�洢�����ݳ���16λ 
	DMA_str.DMA_Mode 					= DMA_Mode_Circular;						//	ѭ��ģʽ 
	DMA_str.DMA_Priority 				= DMA_Priority_High;						//	�����ȼ�
	DMA_str.DMA_FIFOMode 				= DMA_FIFOMode_Disable; 				//	��ʹ��FIFO      
	DMA_str.DMA_FIFOThreshold 		= DMA_FIFOThreshold_1QuarterFull;	//	FIFO��ֵ 
	DMA_str.DMA_MemoryBurst 			= DMA_MemoryBurst_Single;				//	����ͻ�����δ���
	DMA_str.DMA_PeripheralBurst 		= DMA_PeripheralBurst_Single;			//	�洢��ͻ�����δ���
	DMA_Init(DMA1_Stream4, &DMA_str);										//	��ʼ��DMA
			
	DMA_DoubleBufferModeConfig(DMA1_Stream4,(uint32_t)buffer0, DMA_Memory_0);	//	���û�����1
	
	DMA_DoubleBufferModeConfig(DMA1_Stream4,(uint32_t)buffer1, DMA_Memory_1);	//	���û�����1

	DMA_DoubleBufferModeCmd(DMA1_Stream4,ENABLE);										//	����˫����ģʽ
	DMA_ITConfig(DMA1_Stream4,DMA_IT_TC,ENABLE);	//������������ж�

	SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Tx,ENABLE);		//IIS TX DMAʹ��.

	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream4_IRQn;	
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;//��ռ���ȼ�
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;      //��Ӧ���ȼ�
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/**
 *@brief:      mcu_i2s_dma_start
 *@details:    ��ʼDMA����
 *@param[in]   void  
 *@param[out]  ��
 *@retval:     
 */
void mcu_i2s_dma_start(void)
{   	  
	DMA_Cmd(DMA1_Stream4,ENABLE);	// ����DMA TX����
}
/**
 *@brief:      mcu_i2s_dma_stop
 *@details:       ֹͣDMA����
 *@param[in]  void  
 *@param[out]  ��
 *@retval:     
 */
void mcu_i2s_dma_stop(void)
{   	 
	DMA_Cmd(DMA1_Stream4,DISABLE);	//�ر� DMA TX����
}

/**
 *@brief:      mcu_i2s_dma_process
 *@details:    I2Sʹ�õ�DMA�жϴ�������
 *@param[in]   void  
 *@param[out]  ��
 *@retval:     

 λ 19 CT����ǰĿ�꣨����˫������ģʽ�£� (Current target (only in double buffer mode))
��λ��Ӳ���� 1 �����㣬Ҳ��������д�롣
0����ǰĿ��洢��Ϊ�洢�� 0��ʹ�� DMA_SxM0AR ָ��Ѱַ��
1����ǰĿ��洢��Ϊ�洢�� 1��ʹ�� DMA_SxM1AR ָ��Ѱַ��
ֻ�� EN Ϊ��0��ʱ����λ�ſ���д�룬��ָʾ��һ�δ����Ŀ��洢������ʹ��������
�󣬴�λ�൱��һ��״̬��־������ָʾ��Ϊ��ǰĿ��Ĵ洢����
 */
void mcu_i2s_dma_process(void)
{
	if(DMA1_Stream4->CR&(1<<19))
	{
		/*��ǰĿ��洢��Ϊ1�����Ǿ����ÿ���BUFΪ0*/
		fun_sound_set_free_buf(0);
	}
	else
	{
		fun_sound_set_free_buf(1);
	}
}
