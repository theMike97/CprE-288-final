/*
 * main.c
 *
 *  Created on: Mar 29, 2018
 *      Author: mlauderb
 */

#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "Timer.h"
#include "lcd.h"
#include "button.h"
#include "servo.h"
#include "sonar.h"
#include "IR.h"
#include "uart.h"
#include "WiFi.h"
#include "driverlib/interrupt.h"
#include <inc/tm4c123gh6pm.h>

#define max(x,y) ((x) >= (y)) ? (x) : (y);
#define min(x,y) ((x) <= (y)) ? (x) : (y);
#define csc(x) 1/sin((x));
#define PI 3.14159265358979323846

// create obstacle object
struct Obstacle {
    double distance;
    double width;
    double direction;
};

int main(void) {
    // init functions
    lcd_init();

    button_init();

    servo_init(8); // init with bot number
    sonar_init();
    IR_init();
//    uart_init(); // called inside WiFi.c
    WiFi_start("lab9tmN2");

    set_servo_angle0(); // start at 0 degrees
    int i; // for sweep loop
    double angle = 0;
    double sonar_distance = 0;
    double ir_distance = 0;

    char out[50];

    double angle_inc = 2; // turn n degrees
    int edge = 0;
    double angle_width;
    double obstacle_diameter = 0;

    int data_points = 180 / angle_inc;
    int edges[data_points];
    double distance[data_points];
    memset(distance, 100, data_points);

    struct Obstacle obstacles[5];
    struct Obstacle obstacle;
    int obstacle_count = 0;

    double init_angle;

    int on_object = 0;

    while(1) {
//        edge = 0;
        angle_width = 0;
        // Pan IR and sonar
        for (i = 0; i < data_points; i++) {
            sonar_distance = (double) sonar_get_distance(); // get distance measured by sonar
            ir_distance = IR_get_distance(); // get distance measured by IR sensor
            set_servo_offset_angle(angle_inc); // change 2 degrees until 180 degrees (90 times)
            angle += angle_inc;


            // third approach: get all egdes and distances and filter entire array
            distance[i] = sonar_distance;
            edges[i] = ir_distance;

            timer_waitMillis(125); // wait for data to be taken
        }

//        lcd_clear();
//        lcd_printf("Obstacles: %d", obstacle_count);
//        timer_waitMillis(2000);
//        for (i = 0; i < obstacle_count; i++) {
//            lcd_printf("Width: %.1f\nDistance %.1f\nDirection: %.0f", obstacles[i].width, obstacles[i].distance, obstacles[i].direction);
//            timer_waitMillis(2000);
//        }

        double edge_max = 0.0;
        double dist_min = 500.0;

        int i;
        for (i = 0; i < data_points; i++) {
            if (edges[i] > 50.0) {
                edges[i] = 0.0;
                distance[i] = 500.0;
            }
            edge_max = max(edge_max, edges[i]);
            dist_min = min(dist_min, distance[i]);
        }

        char data[20];
        sprintf(data, "edge %.1f, dist %.1f", edge_max, dist_min);v
        uart_sendStr(data);
        timer_waitMillis(2000);

        set_servo_angle0(); // reset servo angle at end of sweep to start over
        angle = 0;
        obstacle_count = 0;
        timer_waitMillis(700); // wait for servo to reset
    }
}

//double get_obstacle_diameter(double edge, double angle_width) {
//    double angle_rad = (PI / 180) * angle_width;
//    double sin_a = sin(angle_rad);
//    double beta = (PI / 2) - (angle_rad / 2);
//    double csc_beta = csc(beta);
//
//    return edge * sin_a * csc_beta * csc_beta;
//}
