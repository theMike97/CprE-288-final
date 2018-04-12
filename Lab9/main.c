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
#define PI 3.14159265358979323846

double get_obstacle_diameter(double edge, double angle_width);

// create obstacle object
struct Obstacle {
    double edge_max;
    double distance;
    double width;
    double direction;
    double diameter;
};

int main(void) {
    // init functions
    lcd_init();

    button_init();

    servo_init(7); // init with bot number
    sonar_init();
    IR_init();
    uart_init(); // called inside WiFi.c
//    WiFi_start("cpre288psk");

    set_servo_angle0(); // start at 0 degrees
    int i; // for sweep loop
    int j; // for noise reduction loop
    double angle = 0.0;
    double sonar_distance = 0.0;
    double ir_distance = 0.0;

//    char out[50];

    double angle_inc = 2; // turn n degrees
    double angle_width = 0.0;

    int data_points = 180 / angle_inc;
    double edges[data_points];
    double new_edges[data_points];
    memset(edges, 0, data_points*sizeof(double));

    struct Obstacle obstacles[5];
    struct Obstacle obstacle;
    int obstacle_count = 0;

    double init_angle = 0.0;

    int on_object;

    // init edge_max and dist_min
    double edge_max;
    double dist_min;

    while(1) {
        angle_width = 0;
        on_object = 0;
        edge_max = 0.0;
        dist_min = 500.0;
        init_angle = 0.0;
        obstacle_count = 0;

        // Pan IR and sonar
        for (i = 0; i < data_points; i++) {
            sonar_distance = sonar_get_distance(); // get distance measured by sonar
            ir_distance = IR_get_distance(); // get distance measured by IR sensor
            set_servo_offset_angle(angle_inc); // change 'angle_inc' degrees until 180 degrees
            angle += angle_inc;

            if (ir_distance < 50.0) { // if edge > 40 cm
                if (on_object == 0) { // if new object
                    uart_sendStr("new obj \n\r");
                    on_object = 1; // set object status
                    init_angle = angle; // set init angle for position
                    obstacle_count++; // increase obstacle count
                } else { // if same object
//                    uart_sendStr("same obj \n\r");
                    angle_width += angle_inc; // increment angle width over object
                }
                edges[i] = sonar_distance; // add sonar_distance to edges array

//                edge_max = max(edge_max, edges[i]);
//                dist_min = min(dist_min, edges[i]);

            } else if (ir_distance > 50.0) { // if edge distance > 40 cm
                if (on_object == 1) { // if same object
                    uart_sendStr("end obj \n\r");
                    // noise reduction loop (1st derivative)
                    for (j = 0; j < data_points; j++) {
                        if (abs(edges[j] - edges[j-1]) > 0.8) {
                            new_edges[j] = 0.0;
                        } else {
                            new_edges[j] = edges[j];
                        }
                        edge_max = max(edge_max, new_edges[j]); // set edge_max against filtered edge array
                        dist_min = min(dist_min, new_edges[j]); // set dist_min against filtered edge array
                    }
                    on_object = 0; // set object status to no object
                    // set obstacle struct fields
                    obstacle.edge_max = edge_max;
                    obstacle.width = angle_width;
                    obstacle.direction = init_angle + (angle_width / 2);
                    obstacle.distance = dist_min;
                    obstacle.diameter = get_obstacle_diameter(edge_max, angle_width);
                    obstacles[obstacle_count - 1] = obstacle;
                }
                // reset edges, new_edges
                memset(new_edges, 0, data_points*sizeof(double));
                memset(edges, 0, data_points*sizeof(double));
                // reset below vars
                angle_width = 0.0;
                edge_max = 0.0;
                dist_min = 500.0;
                init_angle = 0.0;
            }

            timer_waitMillis(40); // wait for data to be taken
        }

        char data[50];
        sprintf(data, "Obstacles: %d\n\r", obstacle_count);
        uart_sendStr(data);
        timer_waitMillis(1000);

        for (i = 0; i < obstacle_count; i++) {
            obstacle = obstacles[i];
            sprintf(data, "edge %.1f, dist %.1f, angle %.1f, width %.1f, diameter %.1f\n\r", obstacle.edge_max, obstacle.distance, obstacle.direction, obstacle.width, obstacle.diameter);
            uart_sendStr(data);
            timer_waitMillis(1000);
        }

        struct Obstacle smallest_obstacle;
        smallest_obstacle = obstacles[0];
        for (i = 1; i < obstacle_count; i++) {
            smallest_obstacle = (smallest_obstacle.diameter < obstacles[i].diameter) ? smallest_obstacle : obstacles[i];
        }
        set_servo_angle(smallest_obstacle.direction);
        timer_waitMillis(2000);
//        char data[20];
//        double d = get_obstacle_diameter(edge_max, angle_width);
//        sprintf(data, "edge %.1f, dist %.1f, angle %.1f, diameter %.1f, num %d\n\r", edge_max, dist_min, angle_width, d, obstacle_count);

//        lcd_printf(data);
//        uart_sendStr(data);
//        timer_waitMillis(2000);

        set_servo_angle0(); // reset servo angle at end of sweep to start over
        angle = 0;
        angle_width = 0;
        obstacle_count = 0;
        timer_waitMillis(800); // wait for servo to reset
    }
}

double get_obstacle_diameter(double edge, double angle_width) {
    /*
     * formula is D = d*sin(alpha)*sec^2(alpha/2) where:
     *
     * D = diameter
     * d = edge_max
     * alpha = angle_width
     */
    double angle_rad = (PI / 180) * angle_width;
    double sin_a = sin(angle_rad);
    double beta = (angle_width / 2) * (PI / 180);
    double cos_beta = cos(beta);

    return edge * sin_a / (cos_beta * cos_beta);
}
