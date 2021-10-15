# SparkIO2

New version of SparkIO to run on a single ESP32.   

Now with conditional compilation, this works with:   

M5Stack Core 2   
Heltec WIFI 32   

And:   
Android app   
IOS app   

#define M5_BRD (or not)   
#define IOS (or not)  

For those that have been following, it now has a single class for talking to the Spark app and the Spark amp   
The code needs refactoring - it is just thrown together from the previous classes with no removal of duplication   


