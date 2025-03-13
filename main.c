// ...existing code...

// Na função main ou em uma função de teste
void test_display() {
    vfd_init();
    HAL_Delay(100);
    
    vfd_clear();
    HAL_Delay(10);
    
    // Testa a exibição em posições específicas
    vfd_print_at(0, 0, "Linha 1 Pos 0");
    vfd_print_at(0, 1, "Linha 2 Pos 0");
    
    // Teste de caracteres individuais
    vfd_set_cursor(10, 0);
    vfd_write_data('A');
    vfd_set_cursor(11, 0);
    vfd_write_data('B');
    vfd_set_cursor(12, 0);
    vfd_write_data('C');
}

// ...existing code...
