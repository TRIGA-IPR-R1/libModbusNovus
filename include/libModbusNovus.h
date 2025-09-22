/*
libModbusNovus is a library to communicate with the NOVUS devices
using MODBUS-RTU (RS-485) on a GNU operating system.
Copyright (C) 2025 Thalles Campagnani

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <chrono>

struct NOVUS_DATA
{
    int   STATE             = -1;   //Status da SPU:
                                    //-1 = Nenhuma tentativa de conexão realizada ainda.
                                    // 0 = Valores lidos com sucesso.
                                    // 1 = Erro ao ler dados.
                                    // 2 = Erro de contexto modbus (provavelmente não existe dispositivo USB).
    std::chrono::system_clock::time_point TIME;
    float   PV              = -1;
};


void libModbusNovus_license();

struct libModbusNovus_private;

class libModbusNovus {
public:
    libModbusNovus(std::string portname);
    ~libModbusNovus();

    bool tryConnect();
    
    //Get the name of the port (who am I?)
    std::string  get_portname();

    //Get just variable
    NOVUS_DATA N2000_get_PV();
    NOVUS_DATA N1500_get_PV();
    NOVUS_DATA N1500ft_get_PV();
    
private:
    libModbusNovus_private* _p;

    std::string stdErrorMsg(std::string functionName, std::string errorMsg, std::string exptionMsg);
    float conv2RegsToFloat(uint16_t data1, uint16_t data2);
    float get_1_DATA_FP(int start_address);
    uint16_t get_1_DATA(int address);
};
