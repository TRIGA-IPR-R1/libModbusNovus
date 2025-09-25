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

#include <libModbusNovus.h>

#include <modbus/modbus-rtu.h>
#include <modbus/modbus.h>

struct libModbusNovus_private {
    std::string portname;
    modbus_t* ctx;
    NOVUS_DATA novusData;
    bool flagNotConnected;//Devido a um erro na biblioteca ModBus na função modbus_free() que causa falha
    //de segmentação, fez-se necessário criar essa flag para as funções da categoria get_data...() consigam
    //saber que o dispositivo não existe para emitir STATE 2 (desconectado)
};

void libModbusNovus_license()
{
    std::cout << "libModbusNovus Copyright (C) 2025 Thalles Campagnani" << std::endl;
    std::cout << "This program comes with ABSOLUTELY NO WARRANTY;" << std::endl;
    std::cout << "This is free software, and you are welcome to redistribute it" << std::endl;
    std::cout << "under certain conditions; For more details read the file LICENSE" << std::endl;
    std::cout << "that came together with the library." << std::endl << std::endl;
}

bool libModbusNovus::tryConnect()
{
    if (this->_p->ctx == nullptr) {
        // Não podemos conectar se a criação do contexto falhou no construtor
        return 1; 
    }

    // Set the slave address
    modbus_set_slave(this->_p->ctx, 0x01);

    if (this->_p->ctx == nullptr || modbus_connect(this->_p->ctx) == -1) 
    {
        std::cerr << stdErrorMsg("tryConnect()","Failed to connect to Modbus device:", modbus_strerror(errno));
        modbus_close(this->_p->ctx);
        //modbus_free(this->_p->ctx); //Bug na biblioteca causa falha de segmentação
        this->_p->flagNotConnected=1;
        return 1;
    }
    else
    {
        std::cout << "Connection successful to: " << get_portname() << std::endl;
        this->_p->flagNotConnected=0;
        return 0;
    } 
}

libModbusNovus::libModbusNovus(std::string portname)
{
    this->_p = new libModbusNovus_private;
    this->_p->portname = portname;
    this->_p->flagNotConnected = true; // Começa como desconectado
    // Apenas crie o contexto. A conexão será feita depois.
    this->_p->ctx = modbus_new_rtu(this->_p->portname.c_str(), 57600, 'N', 8, 1);

    if (this->_p->ctx == nullptr) std::cerr << stdErrorMsg("Constructor", "Failed to create Modbus context", modbus_strerror(errno));
}

libModbusNovus::~libModbusNovus() {
    // Close the Modbus connection
    if (this->_p->ctx) {
        modbus_close(this->_p->ctx);
        modbus_free(this->_p->ctx); // Biblioteca estava com bug, tentativa de ver se foi solucionado
        this->_p->ctx = nullptr;
    }
    delete this->_p;
}

std::string  libModbusNovus::get_portname()    { return this->_p->portname; }

float libModbusNovus::conv2RegsToFloat(uint16_t data1, uint16_t data2)
{
    uint16_t data[2] = {data2, data1};
    float* floatValue = reinterpret_cast<float*>(data);
    return *floatValue;
}

std::string libModbusNovus::stdErrorMsg(std::string functionName, std::string errorMsg, std::string exptionMsg)
{
    std::string msg =   "ERROR in libModbusNovus::" + 
                        functionName +
                        " at " + 
                        get_portname() + 
                        "\n\tError type: " + 
                        errorMsg;
    if (exptionMsg!="") 
    {
        msg +=          "\n\tError code: " + 
                        exptionMsg;
    }
    msg += "\n";
    return msg;
}


/*
N2000
    PV:     0001
    dppo:   0030
        Posição do ponto decimal de PV.
        Faixa: 0 a 3.
        0 → X.XXX;
        1 → XX.XX;
        2 → XXX.X;
        3 → XXXX.
        OBS: para leirura de temperatura, sempre será apenas 1 casa decimal.

N1500FT
    PV_H:   0000
        Vazão instantânea (ponto flutuante – word high) 
    PV_L:   0001
        Vazão instantânea (ponto flutuante – word low)

#N1500
    PV:     0000
    Dp.pos: 0011
        Posição do ponto decimal de PV.
        Faixa: 0 a 5.
        0 → XXXXX;
        1 → XXXXX.X;
        2 → XXXX.XX;
        3 → XXX.XXX;
        4 → XX.XXXX;
        5 → X.XXXXX.
        OBS: para leirura de temperatura, sempre será apenas 1 casa decimal.
*/


NOVUS_DATA libModbusNovus::N2000_get_PV()
{
    // Check if the Modbus context exists
    if (!this->_p->ctx || this->_p->flagNotConnected) {
        std::cerr << stdErrorMsg("get_PV()","Modbus context does not exist","");
        this->_p->novusData.STATE = 2;
        this->_p->novusData.TIME = std::chrono::system_clock::now();
        return this->_p->novusData;
    }

    // Read the register
    int start_address =          0x0001; // Starting address
    int   end_address =          0x0001; // Ending   address
    int num_registers = 1 + end_address - start_address; // Number of registers to read
    uint16_t data[num_registers];
    int result = modbus_read_registers(this->_p->ctx, start_address, num_registers, data);

    if (result == -1) {
        std::cerr << stdErrorMsg("get_PV()","Failed to read data",modbus_strerror(errno));
        this->_p->novusData.STATE = 1;
        this->_p->novusData.TIME = std::chrono::system_clock::now();
        return this->_p->novusData;
    }

    // Convert data in floats variables
    this->_p->novusData.PV       = ((float) data[0])/10;

    this->_p->novusData.STATE           = 0;
    this->_p->novusData.TIME = std::chrono::system_clock::now();
    return this->_p->novusData;
}

NOVUS_DATA libModbusNovus::N1500_get_PV()
{
    // Check if the Modbus context exists
    if (!this->_p->ctx || this->_p->flagNotConnected) {
        std::cerr << stdErrorMsg("get_PV()","Modbus context does not exist","");
        this->_p->novusData.STATE = 2;
        this->_p->novusData.TIME = std::chrono::system_clock::now();
        return this->_p->novusData;
    }

    // Read the register
    int start_address =          0x0000; // Starting address
    int   end_address =          0x0000; // Ending   address
    int num_registers = 1 + end_address - start_address; // Number of registers to read
    uint16_t data[num_registers];
    int result = modbus_read_registers(this->_p->ctx, start_address, num_registers, data);

    if (result == -1) {
        std::cerr << stdErrorMsg("get_PV()","Failed to read data",modbus_strerror(errno));
        this->_p->novusData.STATE = 1;
        this->_p->novusData.TIME = std::chrono::system_clock::now();
        return this->_p->novusData;
    }

    // Convert data in floats variables
    this->_p->novusData.PV       = ((float) data[0])/10;

    this->_p->novusData.STATE           = 0;
    this->_p->novusData.TIME = std::chrono::system_clock::now();
    return this->_p->novusData;
}

NOVUS_DATA libModbusNovus::N1500ft_get_PV()
{
    // Check if the Modbus context exists
    if (!this->_p->ctx || this->_p->flagNotConnected) {
        std::cerr << stdErrorMsg("get_PV()","Modbus context does not exist","");
        this->_p->novusData.STATE = 2;
        this->_p->novusData.TIME = std::chrono::system_clock::now();
        return this->_p->novusData;
    }

    // Read from the register 0x0001 to 0x0012
    int start_address =          0x0000; // Starting address
    int   end_address =          0x0001; // Ending   address
    int num_registers = 1 + end_address - start_address; // Number of registers to read
    uint16_t data[num_registers];
    int result = modbus_read_registers(this->_p->ctx, start_address, num_registers, data);

    if (result == -1) {
        std::cerr << stdErrorMsg("get_PV()","Failed to read data",modbus_strerror(errno));
        this->_p->novusData.STATE = 1;
        this->_p->novusData.TIME = std::chrono::system_clock::now();
        return this->_p->novusData;
    }

    // Convert data in floats variables
    this->_p->novusData.PV       = conv2RegsToFloat(data[ 0], data[ 1]);
    

    this->_p->novusData.STATE           = 0;
    this->_p->novusData.TIME = std::chrono::system_clock::now();
    return this->_p->novusData;
}