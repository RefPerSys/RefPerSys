## Objective

## Dependencies

* libfcgi-dev
* nginx
* spawn-fcgi

`sudo apt install libfcgi-dev nginx spawn-fcgi`

## Configuration
```
    server {
	    listen 9090;
	    server_name localhost;
	    
	    location / {
		root /srv/http/test;
		fastcgi_pass 127.0.0.1:8000;
		include fastcgi_params;
	    }

	    location /css {
		root /srv/http/test;
		add_header Content-Type text/css;
	    }
	    
	    location /js {
		root /srv/http/test;
		add_header Content-Type application/javascript;
	    }
	    
	    location ~ \.css {
		root /srv/http/test;
		add_header Content-Type text/css;
	    }
	    
	    location ~ \.js {
		root /srv/http/test;
		add_header Content-Type application/javascript;
	    }

	    access_log /var/log/nginx/access.log;
	    error_log /var/log/nginx/error.log warn;
    }
```
## Classes
