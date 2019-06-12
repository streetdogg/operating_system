#include "interrupts.h"

void printf(char* str);

InterruptManager::GateDescriptor InterruptManager::interruptDescriptorTable[256];

void InterruptManager::SetInterruptDescriptorTableEntry(
    uint8_t interrupt,
    uint16_t CodeSegment,
    void (*handler)(),
    uint8_t DescriptorPrivilegeLevel,
    uint8_t DescriptorType) {

    const uint8_t IDT_DESC_PRESENT = 0x80;

    interruptDescriptorTable[interrupt].handlerAddressLowBits = ((uint32_t)handler) & 0xFFFF;
    interruptDescriptorTable[interrupt].handlerAddressHighBits = (((uint32_t)handler) >> 16) & 0xFFFF;
    interruptDescriptorTable[interrupt].gdt_codeSegmentSelector = CodeSegment;
    interruptDescriptorTable[interrupt].access = IDT_DESC_PRESENT | DescriptorType | ((DescriptorPrivilegeLevel&3)<<5);
    interruptDescriptorTable[interrupt].reserved = 0;
}
    
InterruptManager::InterruptManager(uint16_t hardwareInterruptOffset, GlobalDescriptorTable *gdt):
picMasterCommand(0x20),
picMasterData(0x21),
picSlaveCommand(0xA0),
picSlaveData(0xA1){
    this->hardwareInterruptOffset = hardwareInterruptOffset;
    uint16_t CodeSegment = gdt->CodeSegmentSelector();
    const uint8_t IDT_INTERRUPT_GATE = 0xE;

    for (uint16_t i = 0; i < 256; i++)
        SetInterruptDescriptorTableEntry(i, CodeSegment, &InterruptIgnore, 0, IDT_INTERRUPT_GATE);

    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0, CodeSegment, &HandleInterruptRequest0x00, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 1, CodeSegment, &HandleInterruptRequest0x01, 0, IDT_INTERRUPT_GATE);

    picMasterCommand.Write(0x11);
    picSlaveCommand.Write(0x11);

    picMasterData.Write(0x20);
    picSlaveData.Write(0x28);

    picMasterData.Write(0x04);
    picSlaveData.Write(0x02);

    picMasterData.Write(0x01);
    picSlaveData.Write(0x01);

    picMasterData.Write(0x00);
    picSlaveData.Write(0x00);

    interruptDescriptorTablePointer idt;
    idt.size = 256 * sizeof(GateDescriptor) - 1;
    idt.base = (uint32_t)interruptDescriptorTable;

    asm volatile("lidt %0" :: "m" (idt));
}

InterruptManager::~InterruptManager() {
    Deactivate();
}

uint16_t InterruptManager::HardwareInterruptOffset()
{
    return hardwareInterruptOffset;
}

void InterruptManager::Activate(){
    asm("sti");
}

void InterruptManager::Deactivate(){
    printf("Deactivating!");
}


uint32_t InterruptManager::HandleInterrupt(uint8_t interrupt, uint32_t esp) {
    printf("Interrupt!\n");
    return esp;
}
