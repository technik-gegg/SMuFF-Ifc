/**
 * SMuFF IFC Firmware
 * Copyright (C) 2019 Technik Gegg
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include <ArduinoJson.h>

void processDuetJson(DynamicJsonDocument doc) {
    /*
    const char* status = doc["status"]; //  I=idle, P=printing from SD card, S=stopped (i.e. needs a reset), C=running config file (i.e starting up), A=paused, D=pausing, R=resuming from a pause, B=busy (e.g. running a macro), F=performing firmware update

    JsonObject coords = doc["coords"];

    JsonArray coords_axesHomed = coords["axesHomed"];
    int coords_axesHomed_0 = coords_axesHomed[0]; 
    int coords_axesHomed_1 = coords_axesHomed[1]; 
    int coords_axesHomed_2 = coords_axesHomed[2]; 

    int coords_wpl = coords["wpl"]; // 1

    JsonArray coords_xyz = coords["xyz"];
    int coords_xyz_0 = coords_xyz[0]; // 100
    int coords_xyz_1 = coords_xyz[1]; // 240
    float coords_xyz_2 = coords_xyz[2]; // 80.19

    JsonArray coords_machine = coords["machine"];
    int coords_machine_0 = coords_machine[0]; // 100
    int coords_machine_1 = coords_machine[1]; // 240
    float coords_machine_2 = coords_machine[2]; // 80.18

    float coords_extr_0 = coords["extr"][0]; // 317.4

    int speeds_requested = doc["speeds"]["requested"]; // 0
    int speeds_top = doc["speeds"]["top"]; // 0

    int currentTool = doc["currentTool"]; // 3

    JsonObject params = doc["params"];
    int params_atxPower = params["atxPower"]; // 0

    JsonArray params_fanPercent = params["fanPercent"];
    int params_fanPercent_0 = params_fanPercent[0]; // 0
    int params_fanPercent_1 = params_fanPercent[1]; // 100
    int params_fanPercent_2 = params_fanPercent[2]; // 0
    int params_fanPercent_3 = params_fanPercent[3]; // 0

    JsonArray params_fanNames = params["fanNames"];
    const char* params_fanNames_0 = params_fanNames[0]; // ""
    const char* params_fanNames_1 = params_fanNames[1]; // ""
    const char* params_fanNames_2 = params_fanNames[2]; // "Licht"
    const char* params_fanNames_3 = params_fanNames[3]; // ""

    int params_speedFactor = params["speedFactor"]; // 100

    int params_extrFactors_0 = params["extrFactors"][0]; // 100

    float params_babystep = params["babystep"]; // -0.01

    int seq = doc["seq"]; // 114

    int sensors_probeValue = doc["sensors"]["probeValue"]; // 1000
    int sensors_fanRPM = doc["sensors"]["fanRPM"]; // 0

    JsonObject temps = doc["temps"];

    JsonObject temps_bed = temps["bed"];
    float temps_bed_current = temps_bed["current"]; // 28.4
    int temps_bed_active = temps_bed["active"]; // 0
    int temps_bed_standby = temps_bed["standby"]; // 0
    int temps_bed_state = temps_bed["state"]; // 2
    int temps_bed_heater = temps_bed["heater"]; // 0

    JsonArray temps_current = temps["current"];
    float temps_current_0 = temps_current[0]; // 28.4
    int temps_current_1 = temps_current[1]; // 28
    int temps_current_2 = temps_current[2]; // 2000
    int temps_current_3 = temps_current[3]; // 2000

    JsonArray temps_state = temps["state"];
    int temps_state_0 = temps_state[0]; // 2
    int temps_state_1 = temps_state[1]; // 2
    int temps_state_2 = temps_state[2]; // 0
    int temps_state_3 = temps_state[3]; // 0

    JsonArray temps_names = temps["names"];
    const char* temps_names_0 = temps_names[0]; // ""
    const char* temps_names_1 = temps_names[1]; // ""
    const char* temps_names_2 = temps_names[2]; // ""
    const char* temps_names_3 = temps_names[3]; // ""

    JsonArray temps_tools_active = temps["tools"]["active"];

    int temps_tools_active_0_0 = temps_tools_active[0][0]; // 0

    int temps_tools_active_1_0 = temps_tools_active[1][0]; // 0

    int temps_tools_active_2_0 = temps_tools_active[2][0]; // 0

    int temps_tools_active_3_0 = temps_tools_active[3][0]; // 0

    JsonArray temps_tools_standby = temps["tools"]["standby"];

    int temps_tools_standby_0_0 = temps_tools_standby[0][0]; // 0

    int temps_tools_standby_1_0 = temps_tools_standby[1][0]; // 0

    int temps_tools_standby_2_0 = temps_tools_standby[2][0]; // 0

    int temps_tools_standby_3_0 = temps_tools_standby[3][0]; // 0

    const char* temps_extra_0_name = temps["extra"][0]["name"]; // "*MCU"
    float temps_extra_0_temp = temps["extra"][0]["temp"]; // 36.9

    long time = doc["time"]; // 428120
    int coldExtrudeTemp = doc["coldExtrudeTemp"]; // 160
    int coldRetractTemp = doc["coldRetractTemp"]; // 90
    const char* compensation = doc["compensation"]; // "Mesh"
    int controllableFans = doc["controllableFans"]; // 13
    int tempLimit = doc["tempLimit"]; // 320
    int endstops = doc["endstops"]; // 3748
    const char* firmwareName = doc["firmwareName"]; // "RepRapFirmware for Duet 2 WiFi/Ethernet"
    const char* geometry = doc["geometry"]; // "cartesian"
    int axes = doc["axes"]; // 3
    int totalAxes = doc["totalAxes"]; // 4
    const char* axisNames = doc["axisNames"]; // "XYZU"
    int volumes = doc["volumes"]; // 2
    int mountedVolumes = doc["mountedVolumes"]; // 1
    const char* name = doc["name"]; // "ZEvolution II"

    JsonObject probe = doc["probe"];
    int probe_threshold = probe["threshold"]; // 1000
    float probe_height = probe["height"]; // 0.75
    int probe_type = probe["type"]; // 9

    JsonArray tools = doc["tools"];

    JsonObject tools_0 = tools[0];
    int tools_0_number = tools_0["number"]; // 0
    const char* tools_0_name = tools_0["name"]; // "SMuFF-0"

    int tools_0_heaters_0 = tools_0["heaters"][0]; // 1

    int tools_0_drives_0 = tools_0["drives"][0]; // 0

    int tools_0_axisMap_0_0 = tools_0["axisMap"][0][0]; // 0

    int tools_0_axisMap_1_0 = tools_0["axisMap"][1][0]; // 1

    int tools_0_fans = tools_0["fans"]; // 1
    const char* tools_0_filament = tools_0["filament"]; // ""

    JsonArray tools_0_offsets = tools_0["offsets"];
    int tools_0_offsets_0 = tools_0_offsets[0]; // 0
    int tools_0_offsets_1 = tools_0_offsets[1]; // 0
    int tools_0_offsets_2 = tools_0_offsets[2]; // 0

    JsonObject tools_1 = tools[1];
    int tools_1_number = tools_1["number"]; // 1
    const char* tools_1_name = tools_1["name"]; // "SMuFF-1"

    int tools_1_heaters_0 = tools_1["heaters"][0]; // 1

    int tools_1_drives_0 = tools_1["drives"][0]; // 0

    int tools_1_axisMap_0_0 = tools_1["axisMap"][0][0]; // 0

    int tools_1_axisMap_1_0 = tools_1["axisMap"][1][0]; // 1

    int tools_1_fans = tools_1["fans"]; // 1
    const char* tools_1_filament = tools_1["filament"]; // ""

    JsonArray tools_1_offsets = tools_1["offsets"];
    int tools_1_offsets_0 = tools_1_offsets[0]; // 0
    int tools_1_offsets_1 = tools_1_offsets[1]; // 0
    int tools_1_offsets_2 = tools_1_offsets[2]; // 0

    JsonObject tools_2 = tools[2];
    int tools_2_number = tools_2["number"]; // 2
    const char* tools_2_name = tools_2["name"]; // "SMuFF-2"

    int tools_2_heaters_0 = tools_2["heaters"][0]; // 1

    int tools_2_drives_0 = tools_2["drives"][0]; // 0

    int tools_2_axisMap_0_0 = tools_2["axisMap"][0][0]; // 0

    int tools_2_axisMap_1_0 = tools_2["axisMap"][1][0]; // 1

    int tools_2_fans = tools_2["fans"]; // 1
    const char* tools_2_filament = tools_2["filament"]; // ""

    JsonArray tools_2_offsets = tools_2["offsets"];
    int tools_2_offsets_0 = tools_2_offsets[0]; // 0
    int tools_2_offsets_1 = tools_2_offsets[1]; // 0
    int tools_2_offsets_2 = tools_2_offsets[2]; // 0

    JsonObject tools_3 = tools[3];
    int tools_3_number = tools_3["number"]; // 3
    const char* tools_3_name = tools_3["name"]; // "SMuFF-3"

    int tools_3_heaters_0 = tools_3["heaters"][0]; // 1

    int tools_3_drives_0 = tools_3["drives"][0]; // 0

    int tools_3_axisMap_0_0 = tools_3["axisMap"][0][0]; // 0

    int tools_3_axisMap_1_0 = tools_3["axisMap"][1][0]; // 1

    int tools_3_fans = tools_3["fans"]; // 1
    const char* tools_3_filament = tools_3["filament"]; // ""

    JsonArray tools_3_offsets = tools_3["offsets"];
    int tools_3_offsets_0 = tools_3_offsets[0]; // 0
    int tools_3_offsets_1 = tools_3_offsets[1]; // 0
    int tools_3_offsets_2 = tools_3_offsets[2]; // 0

    JsonObject mcutemp = doc["mcutemp"];
    float mcutemp_min = mcutemp["min"]; // 30.9
    float mcutemp_cur = mcutemp["cur"]; // 36.9
    float mcutemp_max = mcutemp["max"]; // 37.7

    JsonObject vin = doc["vin"];
    float vin_min = vin["min"]; // 23.5
    float vin_cur = vin["cur"]; // 24.6
    float vin_max = vin["max"]; // 24.9
    */
}