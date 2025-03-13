// ...existing code...

// Função de inicialização corrigida
void vfd_init(void) {
    // Reset do display
    HAL_GPIO_WritePin(VFD_RESET_GPIO_Port, VFD_RESET_Pin, GPIO_PIN_RESET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(VFD_RESET_GPIO_Port, VFD_RESET_Pin, GPIO_PIN_SET);
    HAL_Delay(50);
    
    // Configuração do modo de comunicação
    vfd_write_command(VFD_CMD_DISPLAY_MODE, 0x00);
    HAL_Delay(5);
    
    // Limpa o display
    vfd_clear();
    
    // Define o modo de endereçamento para garantir posicionamento correto
    vfd_write_command(VFD_CMD_ADDRESS_MODE, 0x00); // Modo de endereçamento incremental
    HAL_Delay(5);
    
    // Ativa o display
    vfd_write_command(VFD_CMD_DISPLAY_CONTROL, 0x08 | 0x04); // Display ON, Cursor OFF
    HAL_Delay(5);
}

// Função para posicionar o cursor corretamente
void vfd_set_cursor(uint8_t x, uint8_t y) {
    // O display tem 20 colunas por linha
    uint8_t address = y * 20 + x;
    vfd_write_command(VFD_CMD_SET_ADDRESS, address);
    HAL_Delay(1);
}

// Função corrigida para exibir uma string em uma posição específica
void vfd_print_at(uint8_t x, uint8_t y, char* str) {
    vfd_set_cursor(x, y);
    
    while (*str) {
        vfd_write_data(*str++);
    }
}

// ...existing code...
