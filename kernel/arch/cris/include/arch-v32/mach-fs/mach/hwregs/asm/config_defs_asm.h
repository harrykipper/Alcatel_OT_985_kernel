
#ifndef __config_defs_asm_h
#define __config_defs_asm_h


#ifndef REG_FIELD
#define REG_FIELD( scope, reg, field, value ) \
  REG_FIELD_X_( value, reg_##scope##_##reg##___##field##___lsb )
#define REG_FIELD_X_( value, shift ) ((value) << shift)
#endif

#ifndef REG_STATE
#define REG_STATE( scope, reg, field, symbolic_value ) \
  REG_STATE_X_( regk_##scope##_##symbolic_value, reg_##scope##_##reg##___##field##___lsb )
#define REG_STATE_X_( k, shift ) (k << shift)
#endif

#ifndef REG_MASK
#define REG_MASK( scope, reg, field ) \
  REG_MASK_X_( reg_##scope##_##reg##___##field##___width, reg_##scope##_##reg##___##field##___lsb )
#define REG_MASK_X_( width, lsb ) (((1 << width)-1) << lsb)
#endif

#ifndef REG_LSB
#define REG_LSB( scope, reg, field ) reg_##scope##_##reg##___##field##___lsb
#endif

#ifndef REG_BIT
#define REG_BIT( scope, reg, field ) reg_##scope##_##reg##___##field##___bit
#endif

#ifndef REG_ADDR
#define REG_ADDR( scope, inst, reg ) REG_ADDR_X_(inst, reg_##scope##_##reg##_offset)
#define REG_ADDR_X_( inst, offs ) ((inst) + offs)
#endif

#ifndef REG_ADDR_VECT
#define REG_ADDR_VECT( scope, inst, reg, index ) \
         REG_ADDR_VECT_X_(inst, reg_##scope##_##reg##_offset, index, \
			 STRIDE_##scope##_##reg )
#define REG_ADDR_VECT_X_( inst, offs, index, stride ) \
                          ((inst) + offs + (index) * stride)
#endif

/* Register r_bootsel, scope config, type r */
#define reg_config_r_bootsel___boot_mode___lsb 0
#define reg_config_r_bootsel___boot_mode___width 3
#define reg_config_r_bootsel___full_duplex___lsb 3
#define reg_config_r_bootsel___full_duplex___width 1
#define reg_config_r_bootsel___full_duplex___bit 3
#define reg_config_r_bootsel___user___lsb 4
#define reg_config_r_bootsel___user___width 1
#define reg_config_r_bootsel___user___bit 4
#define reg_config_r_bootsel___pll___lsb 5
#define reg_config_r_bootsel___pll___width 1
#define reg_config_r_bootsel___pll___bit 5
#define reg_config_r_bootsel___flash_bw___lsb 6
#define reg_config_r_bootsel___flash_bw___width 1
#define reg_config_r_bootsel___flash_bw___bit 6
#define reg_config_r_bootsel_offset 0

/* Register rw_clk_ctrl, scope config, type rw */
#define reg_config_rw_clk_ctrl___pll___lsb 0
#define reg_config_rw_clk_ctrl___pll___width 1
#define reg_config_rw_clk_ctrl___pll___bit 0
#define reg_config_rw_clk_ctrl___cpu___lsb 1
#define reg_config_rw_clk_ctrl___cpu___width 1
#define reg_config_rw_clk_ctrl___cpu___bit 1
#define reg_config_rw_clk_ctrl___iop___lsb 2
#define reg_config_rw_clk_ctrl___iop___width 1
#define reg_config_rw_clk_ctrl___iop___bit 2
#define reg_config_rw_clk_ctrl___dma01_eth0___lsb 3
#define reg_config_rw_clk_ctrl___dma01_eth0___width 1
#define reg_config_rw_clk_ctrl___dma01_eth0___bit 3
#define reg_config_rw_clk_ctrl___dma23___lsb 4
#define reg_config_rw_clk_ctrl___dma23___width 1
#define reg_config_rw_clk_ctrl___dma23___bit 4
#define reg_config_rw_clk_ctrl___dma45___lsb 5
#define reg_config_rw_clk_ctrl___dma45___width 1
#define reg_config_rw_clk_ctrl___dma45___bit 5
#define reg_config_rw_clk_ctrl___dma67___lsb 6
#define reg_config_rw_clk_ctrl___dma67___width 1
#define reg_config_rw_clk_ctrl___dma67___bit 6
#define reg_config_rw_clk_ctrl___dma89_strcop___lsb 7
#define reg_config_rw_clk_ctrl___dma89_strcop___width 1
#define reg_config_rw_clk_ctrl___dma89_strcop___bit 7
#define reg_config_rw_clk_ctrl___bif___lsb 8
#define reg_config_rw_clk_ctrl___bif___width 1
#define reg_config_rw_clk_ctrl___bif___bit 8
#define reg_config_rw_clk_ctrl___fix_io___lsb 9
#define reg_config_rw_clk_ctrl___fix_io___width 1
#define reg_config_rw_clk_ctrl___fix_io___bit 9
#define reg_config_rw_clk_ctrl_offset 4

/* Register rw_pad_ctrl, scope config, type rw */
#define reg_config_rw_pad_ctrl___usb_susp___lsb 0
#define reg_config_rw_pad_ctrl___usb_susp___width 1
#define reg_config_rw_pad_ctrl___usb_susp___bit 0
#define reg_config_rw_pad_ctrl___phyrst_n___lsb 1
#define reg_config_rw_pad_ctrl___phyrst_n___width 1
#define reg_config_rw_pad_ctrl___phyrst_n___bit 1
#define reg_config_rw_pad_ctrl_offset 8


/* Constants */
#define regk_config_bw16                          0x00000000
#define regk_config_bw32                          0x00000001
#define regk_config_master                        0x00000005
#define regk_config_nand                          0x00000003
#define regk_config_net_rx                        0x00000001
#define regk_config_net_tx_rx                     0x00000002
#define regk_config_no                            0x00000000
#define regk_config_none                          0x00000007
#define regk_config_nor                           0x00000000
#define regk_config_rw_clk_ctrl_default           0x00000002
#define regk_config_rw_pad_ctrl_default           0x00000000
#define regk_config_ser                           0x00000004
#define regk_config_slave                         0x00000006
#define regk_config_yes                           0x00000001
#endif /* __config_defs_asm_h */
