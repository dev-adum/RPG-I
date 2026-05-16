#include "Bullet.h"

Bullet::Bullet()

  : life(3.f),
    bulletSpeed(0.075f),
    shape({2.f,2.f}),
    direction(0.f, 0.f)
{
}


void Bullet::Load()
{
    
}


void Bullet::Initialize()
{
    shape.setSize({ 2.f, 2.f });
    
}

void Bullet::Update() 
{

}

void Bullet::Draw() 
{

}