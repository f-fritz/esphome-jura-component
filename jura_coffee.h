#include <string>
#include "esphome.h"

class JuraCoffee : public PollingComponent, public UARTDevice {
    
    // sensor objects from esphome
    // used to publish values via sensor_X->publish_state(value);

    Sensor *sensor_num_cup_s {nullptr};
    Sensor *sensor_num_cup_m {nullptr};
    Sensor *sensor_num_cup_l {nullptr};
    Sensor *sensor_num_cup_1 {nullptr};
    Sensor *sensor_num_cup_2 {nullptr};
    Sensor *sensor_num_cup_powder {nullptr};
    Sensor *sensor_num_op_rinses {nullptr};
    Sensor *sensor_num_op_cleanings {nullptr};
    Sensor *sensor_num_op_descalings {nullptr};
    Sensor *sensor_uart_status {nullptr};

public:
    JuraCoffee(UARTComponent *parent, Sensor *sensor1, Sensor *sensor2, Sensor *sensor3, Sensor *sensor4, Sensor *sensor5, Sensor *sensor6, Sensor *sensor7, Sensor *sensor8, Sensor *sensor9, Sensor *sensor10) : UARTDevice(parent), sensor_num_cup_s(sensor1), sensor_num_cup_m(sensor2), sensor_num_cup_l(sensor3), sensor_num_cup_1(sensor4), sensor_num_cup_2(sensor5), sensor_num_cup_powder(sensor6), sensor_num_op_rinses(sensor7), sensor_num_op_cleanings(sensor8), sensor_num_op_descalings(sensor9), sensor_uart_status(sensor10) {}

    // hold numbers to be published

    uint16_t num_cups_s = 0;
    uint16_t num_cups_m = 0;
    uint16_t num_cups_l = 0;
    uint16_t num_cups_1 = 0;
    uint16_t num_cups_2 = 0;
    uint16_t num_cups_powder = 0;
    uint16_t num_ops_rinses = 0;
    uint16_t num_ops_cleanings = 0;
    uint16_t num_ops_descalings = 0;

    void setup() override {
        // ESP_LOGD("main", "setup()");
        this->set_update_interval(300000); // 5 minutes
    }

    void loop() override {
        // ESP_LOGD("main", "loop()");
    }

    void update() override {
        // ESP_LOGD("main", "update()");
        if (fetchCoffeeStats() == 0) {
            publishSensorData();
            // publishUartData(0);
        } else {
            // publishUartData(-1);
        }
    }

private:
    String cmd2jura(String command_raw_str) {
        String response;
        char currentByte = 0;
        uint16_t failed_reading_attempts = 0;

        if (available() <= 0) {
        } else {
            while (available()) {
                // clear any available data in the buffer
                read();
            }
        }

        command_raw_str += "\r\n";
        for (int i = 0; i < command_raw_str.length(); i++) {
            for (int bitPosition = 0; bitPosition < 8; bitPosition += 2) {
                char byteToSend = 255;
                bitWrite(byteToSend, 2, bitRead(command_raw_str.charAt(i), bitPosition));
                bitWrite(byteToSend, 5, bitRead(command_raw_str.charAt(i), bitPosition + 1));
                write(byteToSend);
            }
            delay(8);
        }

        uint8_t bit_counter = 0;

        while (!response.endsWith("\r\n")) {
            if (available()) {
                byte receivedByte = read();
                bitWrite(currentByte, bit_counter, bitRead(receivedByte, 2));
                bitWrite(currentByte, bit_counter + 1, bitRead(receivedByte, 5));
                bit_counter += 2;

                if (bit_counter >= 8) {
                    bit_counter = 0;
                    response += currentByte;
                }
            } else {
                failed_reading_attempts++;
                delay(10);
            }

            if (response.length() > 200 || failed_reading_attempts > 500) {
                if (response.length() > 200) {
                    ESP_LOGD("main", "Unexpected: response length > 200");
                }
                if (failed_reading_attempts > 500) {
                    ESP_LOGD("main", "Unexpected: failed reading attempts > 500");
                }
                // Add an error message or log an error
                return "";
            }
        }

        return response.substring(0, response.length() - 2);
    }

    int parseHexValue(String input, int startIndex, int endIndex) {
        String substring = input.substring(startIndex, endIndex);
        return static_cast<uint16_t>(strtol(substring.c_str(), NULL, 16));
    }

    int fetchCoffeeStats() {
        // read values from ROM
        String result = cmd2jura("RT:0000");

        if (result.length() < 1) || (result.length() > 60) {
            return -1;
        }

        ESP_LOGD("main", "  result %s", result);

        num_cups_1 = parseHexValue(result, 3, 7);
        num_cups_2 = parseHexValue(result, 7, 11);
        num_cups_s = parseHexValue(result, 11, 15);
        num_cups_m = parseHexValue(result, 15, 19);
        num_cups_l = parseHexValue(result, 19, 23);
        num_ops_descalings = parseHexValue(result, 23,27);
        num_cups_powder = parseHexValue(result, 27,31);
        num_ops_rinses = parseHexValue(result, 31, 35);
        num_ops_cleanings = parseHexValue(result, 35,39);

        return 0;
    }

    void publishSensorData() {
        if (sensor_num_cup_s != nullptr) sensor_num_cup_s->publish_state(num_cups_s);
        if (sensor_num_cup_m != nullptr) sensor_num_cup_m->publish_state(num_cups_m);
        if (sensor_num_cup_l != nullptr) sensor_num_cup_l->publish_state(num_cups_l);
        if (sensor_num_cup_1 != nullptr) sensor_num_cup_1->publish_state(num_cups_1);
        if (sensor_num_cup_2 != nullptr) sensor_num_cup_2->publish_state(num_cups_2);
        if (sensor_num_cup_powder != nullptr) sensor_num_cup_powder->publish_state(num_cups_powder);
        if (sensor_num_op_rinses != nullptr) sensor_num_op_rinses->publish_state(num_ops_rinses);
        if (sensor_num_op_cleanings != nullptr) sensor_num_op_cleanings->publish_state(num_ops_cleanings);
        if (sensor_num_op_descalings != nullptr) sensor_num_op_descalings->publish_state(num_ops_descalings);
    }

    void publishUartData(int state) {
        if (sensor_uart_status != nullptr) sensor_uart_status->publish_state(state);
    }
};