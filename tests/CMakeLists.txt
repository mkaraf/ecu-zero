add_executable(test_isotp_pci test_isotp_pci.c)
target_link_libraries(test_isotp_pci PRIVATE ecu_core unity)
add_test(NAME isotp_pci COMMAND test_isotp_pci)
