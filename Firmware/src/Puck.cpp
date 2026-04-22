#include "Puck.h"

#include <stdio.h>
#include <stdint.h>
#include <string>
#include "images/images.h"
#include "../inc/ST7735.h"

extern Puck pArray[];

Puck::Puck(float startX, float startY, string c, uint8_t puckid){
    id = puckid;
    oldX=startX;
    oldY=startY;
    x=startX;
    y=startY;
    speed=0.0;
    vX=0.0;
    vY=0.0;
    isMoving=false;
    color = c;
    if(color=="Red"){
        image = Red_Puck;
    }
    else if(color=="Blue"){
        image = Blue_Puck;
    }
    erase = Puck_Erase;
    w=15;
    h=15;
}

void Puck::set_Velocity(float velX, float velY){
    isMoving=true;
    speed = sqrt(velX * velX + velY * velY);
    vX=velX/speed;
    vY=velY/speed;
}

int8_t Puck::check_Collisions(uint8_t puckId){
    for(int i=0;i<(int)speed;i++){
        x+=vX;
        y+=vY;
        if(x<=0)return -2;
        if(x+15>=127)return -3;
        if(y<=0)return -4;
        if(y>=125)return -5;
        for(int i=0;i<6;i++){
            if(i==puckId)continue;
            uint8_t xDist = (abs((x+7)-(pArray[i].x+7)));
            uint8_t yDist = (abs((y+7)-(7+pArray[i].y)));
            if((xDist * xDist)+(yDist*yDist)<(225)){
                x -= vX;
                y -= vY;
                return i;
            }
        }
    }
    return -1; 
}

void Puck::process_Collision(uint8_t firstId, uint8_t secondId, float speed){
    //Find the vector between the two centers of the puck 
    float vectorX = ((pArray[firstId].x-pArray[secondId].x));
    float vectorY = ((pArray[firstId].y-pArray[secondId].y));
    float vector = (float)sqrt(vectorX*vectorX + vectorY*vectorY);
    //Normalize it 
    vectorX/=vector;
    vectorY/=vector;
    //Get the part of each velocity that points toward the vector
    float vP1 = pArray[firstId].vX * vectorX + pArray[firstId].vY * vectorY;
    float vP2 = pArray[secondId].vX * vectorX + pArray[secondId].vY * vectorY;
    
    float vP1_new = vP2;
    float vP2_new = vP1;

    float vP2x = pArray[secondId].vX + (vP2_new-vP2) * vectorX;
    float vP2y = pArray[secondId].vY + (vP2_new-vP2) * vectorY;
    pArray[secondId].set_Velocity(vP2x, vP2y);
    pArray[secondId].speed = pArray[firstId].speed;

    float vP1x = pArray[firstId].vX + (vP1_new-vP1) * vectorX;
    float vP1y = pArray[firstId].vY + (vP1_new-vP1) * vectorY;
    pArray[firstId].set_Velocity(vP1x, vP1y);
}

void Puck::process_Movement(){
    if(speed==0){
        isMoving=false;
    }
    oldX=x;
    oldY=y;
    
    int8_t collisionId = check_Collisions(id);

    if(collisionId==-2){
        x=0;
        vX*=-1;
    }
    else if(collisionId==-3){
        x=112;
        vX*=-1;
    }
    else if(collisionId==-4){
        y=0;
        vY*=-1;
    }
    else if(collisionId==-5){
        y=124;
        vY*=-1;
    }
    else if(collisionId!=-1){
        process_Collision(id,collisionId,speed);
    }

    speed -= 0.25f;
    if(speed < 0){
        speed = 0;
    }
}