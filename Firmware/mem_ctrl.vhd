----------------------------------------------------------------------------------
-- Create Date:    19:01:23 02/24/2022 
-- Design Name:    Alirio Oliveira
-- Module Name:    top_level - Behavioral 
-- Project Name:   SD2SMS
-- Target Devices: Master System
-- Tool versions:  14.7
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity mem_ctrl is
    Port ( sms_a : in  STD_LOGIC_VECTOR (15 downto 0);
           sms_d : inout  STD_LOGIC_VECTOR (7 downto 0);
           sms_we : in  STD_LOGIC;
           sms_ce : in  STD_LOGIC;
           sms_oe : in  STD_LOGIC;
           sms_rst : in  STD_LOGIC;
		   sms_M8_B : in  STD_LOGIC;
           clk_in : in  STD_LOGIC;
			  
			  -- SPI CTRL
			  spi_turbo     : out  std_logic;
			  spi_cs_out     : out  std_logic;
			  spi_rw_out      : out std_logic;
			  spi_busy_in     : in  std_logic;
			  -- SPI DATA
			  tx_data_out     : out std_logic_vector(7 downto 0);
			  rx_data_in	   : in std_logic_vector(7 downto 0);
			  
			  -- SRAM(ROM) data
			  rom_data : inout  STD_LOGIC_VECTOR (7 downto 0);
           rom_ce0  : out  STD_LOGIC;  
			  rom_ce1  : out  STD_LOGIC;   			  
			  rom_we   : out  STD_LOGIC;
			  rom_addr : out  STD_LOGIC_VECTOR (18 downto 0);
			  
			  ram_ce : out  STD_LOGIC;
           ram_we : out  STD_LOGIC;
           ram_a14 : out  STD_LOGIC
			  );
end mem_ctrl;

architecture Behavioral of mem_ctrl is
	type StateType is(
		S_IDLE,
		S_READ,
		S_NOP,
		S_WRITE
	);
	signal state			: StateType;
	signal state_next		: StateType;
	
-- Z80
	signal z80_a_sync 	: std_logic_vector(2 downto 0);
	signal z80oe_sync		: std_logic;
	signal z80we_sync		: std_logic;

-- SD
	CONSTANT SDCS         : integer := 0;
	CONSTANT KOREN_MAP    : integer := 1;
	CONSTANT START	       : integer := 2;
	CONSTANT TURBO	       : integer := 3;
	signal Cfg            : std_logic_vector(3 downto 0);
	signal Cfg_next       : std_logic_vector(3 downto 0);
	
-- new logic all in one
	signal romSlot2_s		: std_logic_vector(5 downto 0);
	signal romSlot1_s		: std_logic_vector(5 downto 0);
	signal mapAddr_s		: std_logic_vector(5 downto 0);
	signal romWrEn_s	 	:	std_logic;
	signal romce_s		 	:	std_logic;
	signal slot			 	: std_logic_vector(1 downto 0);
		
	CONSTANT SLOT0     	: std_logic_vector(1 downto 0) := "00";
	CONSTANT SLOT1     	: std_logic_vector(1 downto 0) := "01";
	CONSTANT SLOT2     	: std_logic_vector(1 downto 0) := "10";
	signal slot1_access	: std_logic;
	signal slot2_access	: std_logic;
	
	signal sd_read 		: std_logic;
	signal sd_rcfg			: std_logic;
	signal sd_write		: std_logic;
	signal sd_wcfg			: std_logic;
	
	signal rom_write		: std_logic;
	signal mapram_ce		: std_logic;
	signal mapram_we		: std_logic;
	signal ramEn_s			:	std_logic;
	
	signal fuck		: std_logic;
	
begin	
-- Not Sync
-- 16k Banks
	rom_addr(13 downto 0)	<= sms_a(13 downto 0);
-- Bank select	
	rom_addr(18 downto 14)	<= mapAddr_s(4 downto 0);
-- Enable Bootram when slot 0 and running menu, else user mapper					
	
	-- mudei para adequar a FM1808
	fuck	<=sms_ce when sms_a(15 downto 14) = SLOT0 and Cfg(START) = '1' else mapram_ce; -- Slot 0
	ram_ce	<= fuck or (sms_oe and sms_we);
	
-- Enable writer on Bootram when slot 0 and running menu(cart start on running menu) and mapper RomWrEn enable, else user mapper	
	ram_we	<= sms_we when sms_a(15 downto 14) = SLOT0 and Cfg(START) = '1' and romWrEn_s = '1' else mapram_we;
-- In Menu(1) mapper lower Bootram on slot 0	
	ram_a14	<= Cfg(START);
-- In Menu, direct enable write on rom(ram)game space chip						
	rom_we	<= sms_we when Cfg(START) = '1' else '1'; -- Slot 2

-- Sync	
-- Z80
  
-- Slots, helps
	slot			<= z80_a_sync(2 downto 1);
	
	slot1_access	<= '1' 		when slot = SLOT1 and sms_ce = '0' and Cfg(START) = '1' else '0';
--	Slot 2 acesso
	--slot2_access	<= '1' 		when sms_a(15 downto 14) = SLOT2 and sms_ce = '0' else '0';
	slot2_access	<= '1' 		when sms_M8_B = '0' else '0';
		
	rom_write		<= '1'		when slot2_access = '1' and z80we_sync = '0' else '0';
	rom_data		<= sms_d	when rom_write = '1' else (others => 'Z');
	
	sd_read			<= '1' 		when slot1_access = '1' and z80oe_sync = '0' and z80_a_sync(0) = '0' else '0'; -- USO A2, $4000
	sd_rcfg			<= '1' 		when slot1_access = '1' and z80oe_sync = '0' and z80_a_sync(0) = '1' else '0'; -- USO A2, $4004
	
	sd_write		<= '1' 		when slot1_access = '1' and z80we_sync = '0' and z80_a_sync(0) = '0' else '0'; -- USO A2, $4000
	sd_wcfg			<= '1' 		when slot1_access = '1' and z80we_sync = '0' and z80_a_sync(0) = '1' else '0'; -- USO A2, $4004
-- Put sms on ZZZ or
-- Send to SMS chip rom(ram game) data only when on slot 2	
-- Send to SMS data from sd
-- Send to SMS status from spi and cfg
-- Put sms on ZZZ.
	sms_d				<= (others => 'Z') 				when sms_oe  = '1' or sms_ce = '1' else
							rom_data 					when romce_s = '0' else -- read rom(ram game) slot 2
							rx_data_in					when sd_read = '1' else -- SLOT1 A0
							spi_busy_in  & "000" & Cfg  when sd_rcfg = '1' else -- SLOT1 A1							
							(others => 'Z'); 													
		
-- sd conections
	spi_cs_out  <= Cfg(SDCS);	
	spi_turbo	<= Cfg(TURBO);
	process(state, 
			  sd_read, sd_rcfg, sd_write, sd_wcfg,
			  spi_busy_in,
			  z80oe_sync, z80we_sync)
	begin
		state_next	<= state;
		
		spi_rw_out		<= '0';
		tx_data_out		<= (others => '1');
		Cfg_next			<= Cfg;
				
		case state is
		when S_IDLE	=>	
			if(   sd_read  = '1')then
				spi_rw_out		<= '1';
				state_next		<= S_NOP;				
			
			elsif(sd_rcfg = '1') then
				state_next		<= S_READ;	
				
			elsif(sd_write = '1')then
				tx_data_out	<= sms_d;
				spi_rw_out	<= '1';	
				state_next	<= S_WRITE;	
				
			elsif(sd_wcfg  = '1')then
				Cfg_next	<= sms_d(3 downto 0);
			
			end if;
		when S_NOP =>	
			spi_rw_out		<= '1';	
			state_next		<= S_READ;			
		when S_READ =>	
			if(sd_read = '0') then
				state_next		<= S_IDLE;
			end if;
			
		when S_WRITE =>
			spi_rw_out	<= '1';
			tx_data_out	<= sms_d;
			state_next	<= S_IDLE;
			
		end case;
	end process;

-- Sync sms sinais process				
	process(clk_in, sms_rst)
	begin
		if(sms_rst = '0') then
			state		<= S_IDLE;
			Cfg			<= "0101"; -- TURBO OFF, inMenu, KOREN_MAP disable, sd disable
			
			z80oe_sync	<= '1';
			z80_a_sync	<= (others=>'0');
			z80we_sync	<= '1';
			
		elsif(falling_edge(clk_in)) then	
			state		<= state_next;
			Cfg			<= Cfg_next;
			
			z80oe_sync	<= sms_oe;
			z80we_sync	<= sms_we;
			z80_a_sync	<= sms_a(15 downto 14) & sms_a(2);			
			 
		end if;
	end process;
	
	-- Registros
	registros : process(sms_we, sms_ce, sms_rst, sms_a)
	begin
		
		if(sms_rst = '0') then
			romSlot2_s 	<= "000010";
			romSlot1_s 	<= "000001";
			romWrEn_s <= '0';
			ramEn_s	<= '0';
		elsif(falling_edge(sms_we)) then
			if(sms_ce = '0') then
				case sms_a is	
				when x"A000" => -- Koren map
					if(Cfg(KOREN_MAP) = '1') then
						romSlot2_s  <= sms_d(5 downto 0);
					end if;
				when x"FFFE" => -- Sega map slot 1
					romSlot1_s	<= sms_d(5 downto 0);
				when x"FFFF" => -- Sega map slot 2
					romSlot2_s  <= sms_d(5 downto 0);					
				when x"FFFC" => -- Sega map save enable
					romWrEn_s	<= sms_d(7);	
					ramEn_s		<= sms_d(3);
				when others =>
					null;
					
				end case;
			end if;
		end if;
	end process;
		
	-- Select correct ROM chip, lower or upper
	rom_ce0	<= romce_s when mapAddr_s(5) = '0' else '1';
	rom_ce1	<= romce_s when mapAddr_s(5) = '1' else '1';
	
	process(sms_a, sms_ce)
	begin
		mapram_ce	<= '1';
		mapram_we	<= '1';
		romce_s		<= '1';
		case sms_a(15 downto 14) is
		when "00" =>
			if(Cfg(START) = '0') then
				romce_s	<= sms_ce;
			end if;
		when "01" =>		
			if(Cfg(START) = '0') then
				romce_s	<= sms_ce;
			end if;
		when "10" =>
			if(ramEn_s = '1' ) then
				mapram_ce	<= sms_ce;
				mapram_we	<= sms_we;
			else
				romce_s		<= sms_ce;
			end if;
			
		when others =>
			mapram_ce	<= '1';
			mapram_we	<= '1';
			romce_s		<= '1';
		end case;	
	end process;
	
	-- Bancos / slots 
	bancos : process(sms_a)
	begin
		mapAddr_s	<= (others => '0');
		case sms_a(15 downto 14) is
		when "01" => -- 4000
			mapAddr_s	<= romSlot1_s;
		when "10" => -- 8000
			mapAddr_s	<= romSlot2_s;			
		when others =>
			mapAddr_s	<= (others => '0');
		
		end case;
	end process;
		
end Behavioral;

