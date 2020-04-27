/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include "project.h"

uint8 ByteCount = 0;
uint8 firstbyte = 0;
uint8 CommandReady = 0;
uint8 Count = 0;
uint8 Status;

uint8 StepperMotorDone = 1;

uint8 ReceivedBuffer[66];
uint8 TransmitBuffer[66];

struct command_protocol
{
    uint8 packet_size;
    uint8 command;
    uint8 buffer[64];
} Command_Packet;

struct command_protocol Transmit_Packet;

CY_ISR(ByteReceived)
{ 
    LEDDrive_Write(1);
    ReceivedBuffer[ByteCount++] = (uint8) (LabVIEW_UART_GetByte()&0x00ff);
    if(firstbyte == 0)
    {
        ByteCounter_WriteCompare(ReceivedBuffer[0]);
        firstbyte = 1;
   
    }
    Count = ByteCounter_ReadCounter();
    Status = ByteCounter_ReadStatusRegister();
    LEDDrive_Write(0);
    
}

CY_ISR(CommandReceived)
{
    uint8 i;
    
    if(CommandReady == 0)
    {
        Command_Packet.packet_size = ReceivedBuffer[0];
        Command_Packet.command = ReceivedBuffer[1];
    
        for(i=0;i<(Command_Packet.packet_size - 2);i++)
        {
            Command_Packet.buffer[i] =  ReceivedBuffer[i+2];
        }
    
        firstbyte = 0;
        CommandReady = 1;
        ByteCount = 0;
    }
    ByteCounter_ReadStatusRegister();
    LEDDrive_Write(0);
}

CY_ISR(StepperInterrupt)
{
    StepperStart_Write(0);
    StepperMotorDone = 1;
    StepperCount_ReadStatusRegister();
}


int main(void)
{
    uint8 i;
    
    StepperStart_Write(0);
    
    UARTReset_Write(0);
    StepCountReset_Write(0);
    ByteCountReset_Write(0);
    
    LabVIEW_UART_Start();
    StepperStep_Start();
    
    StepperStep_WritePeriod((uint32)4294967296);
    
    
    
    ByteCounter_Start();
    ByteCounter_Enable();
    ByteCounter_SetInterruptMode(ByteCounter_STATUS_CMP_INT_EN_MASK);
    
    StepperCount_Start();
    StepperCount_Enable();
    StepperCount_SetInterruptMode(ByteCounter_STATUS_CMP_INT_EN_MASK);
       
    ByteReceived_Start();
    ByteReceived_StartEx(ByteReceived);
    ByteReceived_Enable();
    
    CommandReceived_Start();
    CommandReceived_StartEx(CommandReceived);
    CommandReceived_Enable();
    
    StepperInterrupt_Start();
    StepperInterrupt_StartEx(StepperInterrupt);
    StepperInterrupt_Enable();
       
    
    CyGlobalIntEnable; /* Enable global interrupts. */

    /* Place your initialization/startup code here (e.g. MyInst_Start()) */

    //LabVIEW_UART_PutString("PSOC Started\r\n");
    
    for(;;)
    {
        /* Place your application code here. */
        
        if(CommandReady)
        {
            Transmit_Packet.command = Command_Packet.command;
            TransmitBuffer[1] = Transmit_Packet.command;
            Transmit_Packet.packet_size = Command_Packet.packet_size;
            TransmitBuffer[0] = Transmit_Packet.packet_size;

            switch(Command_Packet.command)
            {
                case 0: //Motor Kill
                    for(i=0;i<(Transmit_Packet.packet_size-2);i++)
                    {
                            Transmit_Packet.buffer[i] = Command_Packet.buffer[i];
                            TransmitBuffer[i+2] = Transmit_Packet.buffer[i];
                    }
                    CommandReady = 0;
                    
                    StepperStart_Write(Command_Packet.buffer[0]);
                    
                break;
                case 1: //Set Motor Direction
                    for(i=0;i<(Transmit_Packet.packet_size-2);i++)
                    {
                            Transmit_Packet.buffer[i] = Command_Packet.buffer[i];
                            TransmitBuffer[i+2] = Transmit_Packet.buffer[i];
                    }
                    CommandReady = 0;
                    
                    StepperDirection_Write(Command_Packet.buffer[0]);
                    
                break;
                case 2: //Set Stepper Motor Speed
                    for(i=0;i<(Transmit_Packet.packet_size-2);i++)
                    {
                            Transmit_Packet.buffer[i] = Command_Packet.buffer[i];
                            TransmitBuffer[i+2] = Transmit_Packet.buffer[i];
                    }
                    CommandReady = 0;
                    
                    StepperStep_WritePeriod((int32)((Command_Packet.buffer[3])<<24)|((Command_Packet.buffer[2]<<16)|(Command_Packet.buffer[1])<<8)|(Command_Packet.buffer[0]&0x00FF));
                    
                break;
                case 3: //Set Stepper Motor Steps
                    for(i=0;i<(Transmit_Packet.packet_size-2);i++)
                    {
                            Transmit_Packet.buffer[i] = Command_Packet.buffer[i];
                            TransmitBuffer[i+2] = Transmit_Packet.buffer[i];
                    }
                    CommandReady = 0;
                    
                    StepperCount_WriteCompare((int32)((Command_Packet.buffer[3])<<24)|((Command_Packet.buffer[2]<<16)|(Command_Packet.buffer[1])<<8)|(Command_Packet.buffer[0]&0x00FF));
                    StepperStart_Write(1);
                    StepperMotorDone = 0;
                break;
                default:
                break;
            }
            LabVIEW_UART_PutArray(TransmitBuffer,Transmit_Packet.packet_size);
        }
        
                        
    }
}

/* [] END OF FILE */
