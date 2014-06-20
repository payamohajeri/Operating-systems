gcc Client.c -o client -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/include/libgtop-2.0 -lgtop-2.0 -lglib-2.0 -lpthread

gcc MidServer.c -o mid -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/include/libgtop-2.0 -lgtop-2.0 -lglib-2.0 -lpthread

gcc Server.c -o server -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/include/libgtop-2.0 -lgtop-2.0 -lglib-2.0 -lpthread