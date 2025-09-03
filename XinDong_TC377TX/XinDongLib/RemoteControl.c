#include "RemoteControl.h"

#include "movements.h"

#define CACHE_SIZE      256
#define PACKAGE_HEAD    0xFF
#define PACKAGE_TAIL    0xFE

uint8 _cache[CACHE_SIZE];
uint8 _parser[CACHE_SIZE];
char  _package[CACHE_SIZE];

uint16 _head, _tail;
float _servo, _motor;

void RemoteControl_Init(){
    _head = 0;
    _tail = 0;
}

void RemoteControl_Load(uint8 *_ptr, uint16 _length){
    // load bytes from cache
    _tail = (_tail + _length) % CACHE_SIZE;
    for(int i = 0;i < _length;i++){
        _cache[(_head + i) % CACHE_SIZE] = _ptr[i];
    }
}

void RemoteControl_Detect(){
    uint16 _p = _head;
    uint16 _q;
    uint16 _length = 0;
    while(_p != _tail){
        if(_cache[_p] == PACKAGE_HEAD)
            break;

        _p++;
        _p %= CACHE_SIZE;
    }
    if(_p != _tail){
        uint16 _temp = 0;
        _q = (_p+1) % CACHE_SIZE;

        while(_q != _tail){
            if(_cache[_q] == PACKAGE_HEAD){
                _length = _temp;
                break;
            }

            _q++;
            _q %= CACHE_SIZE;
            _temp++;
        }
    }

    if(_length){
        for(int i = 1;i <= _length;i++){
            _package[i - 1] = _cache[(_p+i) % CACHE_SIZE];
        }
        _package[_length] = 0;
    }

    sscanf("%f,%f", &_servo, &_motor);

    Motor_Set(_motor);
    Servo_Set(_servo);
}
