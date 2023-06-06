----------------------------------------------------------------------------------
-- Create Date:    19:01:23 02/24/2022 
-- Design Name:    Alirio Oliveira
-- Module Name:    top_level - Behavioral 
-- Project Name:   SD2SMS
-- Target Devices: Master System
-- Tool versions:  14.7
----------------------------------------------------------------------------------
-- In MENU CART
-- Slot 0 16kB Boot FRAM
-- Slot 1      SPI R/W
-- Slot 2 16kB Banked SRAM
------------------------------\
-- FRAM						  |
------------------------------|
-- $0000-$3FFF | SALVE REGION |
------------------------------|
-- $4000-$7FFF | BOOT REGION  |
------------------------------/


library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity top_level is
    Port ( 
			  -- Sinais de inout do Master System
			sms_rst : 	in  STD_LOGIC;
			sms_adr : 	in  STD_LOGIC_VECTOR (15 downto 0);
			sms_dat : 	inout  STD_LOGIC_VECTOR (7 downto 0);
			sms_ce : 		in  STD_LOGIC;
			sms_oe : 		in  STD_LOGIC;
			sms_we : 		in  STD_LOGIC;
			sms_M8_B : 	in  STD_LOGIC;
			
				-- Sinais da Memoria SRAM 512KB
			sram_adr : 	out  STD_LOGIC_VECTOR (18 downto 0);
			sram_dat : 	inout  STD_LOGIC_VECTOR (7 downto 0);
			sram_ce0 : 	out  STD_LOGIC;
			sram_ce1 : 	out  STD_LOGIC;
			sram_we : 	out  STD_LOGIC;
				-- Sinais da FRAM de 32KB
			fram_ce : 	out  STD_LOGIC;
			fram_we : 	out  STD_LOGIC;
			fram_banksel : out  STD_LOGIC; -- Seleçao 16 + 16 KB
				-- Sinal de Clock (indefinido)
			clk_in : 		in  STD_LOGIC; -- 50MHz
			  -- Sinais do SDCARD
           -- SD Card 
			  spiClk_out     : buffer   std_logic; 
			  spiData_out    : out   std_logic;
			  spiData_in     : in    std_logic;
			  sdCS_out      : out   std_logic
			  );			  
end top_level;

architecture Behavioral of top_level is
		
	-- mem_ctrl to sdcard
	signal ctrl_sdcs		: std_logic;
	signal sd_rw			: std_logic;
	
	signal ss				: std_logic_vector(0 downto 0);
	signal spi_cs			: std_logic;
	signal sd_busy			: std_logic;
	signal rx_data			: std_logic_vector(7 downto 0);
	signal tx_data			: std_logic_vector(7 downto 0);
	
	signal turbo			: std_logic;
	signal spi_speed		: integer RANGE 1 to 63;
	-- clock; o_sclk frequency = i_clk/(2*CLK_DIV)
	-- 50.000.000 / (2*63) = 396.85 KHZ
	constant spi_slow		: integer := 63;
	-- 50.000.000 / (2*1) = 25 MHZ
	constant spi_fast		: integer := 1;
begin	
			
	ctrl : entity work.mem_ctrl port map ( 
		sms_a 		=> sms_adr,
		sms_d 		=> sms_dat,
		sms_we 		=> sms_we,
		sms_ce 		=> sms_ce,
		sms_oe 		=> sms_oe,
		sms_rst 	=> sms_rst,
		sms_M8_B	=> sms_M8_B,
		clk_in		=> clk_in,
		
		-- SPI control
		spi_turbo		=> turbo,
		spi_cs_out		=> ctrl_sdcs,		-- SD enable
		spi_rw_out		=> sd_rw,			-- SPI start r/w
		spi_busy_in		=> sd_busy,			-- SPI busy signal		
		-- SPI data
		tx_data_out		=> tx_data,			-- SPI data to write
		rx_data_in		=> rx_data,			-- SPI data to read
		
		-- sram data
		rom_data		=> sram_dat,
		rom_addr		=> sram_adr,
		-- ctrl out
		rom_ce0 		=> sram_ce0,	
		rom_ce1 		=> sram_ce1,	
		ram_ce 		=> fram_ce,
		rom_we 		=> sram_we,
		ram_we 		=> fram_we,
		
		-- ctrl out ram bank
		ram_a14 		=> fram_banksel
		
	);
			
	sdCS_out	<= spi_cs or ctrl_sdcs;
	spi_speed	<= spi_slow when turbo = '0' else spi_fast;
	-- spi_cs		<= ss(0);
	-- -- SPI master
	-- spi_master : entity work.spi_master
	-- generic map (
	-- 	--slaves	=> 1,
	-- 	d_width	=> 8
	-- )
	-- port map ( 
	-- 	reset_n     => sms_rst,
	-- 	clock       => clk_in,
	-- 	enable		=> sd_rw,
	-- 	cpol		=> '0',
	-- 	cpha		=> '0',
	-- 	cont		=> '0',
	-- 	clk_div		=> spi_speed,
	-- 	addr		=> 0,
	-- 	tx_data		=> tx_data,
	-- 	miso		=> spiData_in,
	-- 	sclk		=> spiClk_out,
	-- 	ss_n		=> ss,
	-- 	mosi		=> spiData_out,
	-- 	busy		=> sd_busy,
	-- 	rx_data		=> rx_data			
	-- );
		
	spi_controller : entity work.spi_controller
	generic map (
		N		=> 8,
		CLK_DIV	=> 63
	)
	port map(
		i_clk 			  => clk_in,
		i_rstb            => sms_rst,
		i_tx_start        => sd_rw,
		o_tx_end          => sd_busy,
		i_data_parallel   => tx_data,
		o_data_parallel   => rx_data,			
		o_sclk            => spiClk_out,
		o_ss              => spi_cs,
		o_mosi            => spiData_out,
		i_miso            => spiData_in,
		clk_divv		  => spi_speed
	
	);
	
end Behavioral;

