# **Client-Server in C**
## **Socket-based, client-servers program to locally or remotely through _TCP_ run bash commands or manage files**
### **Possible actions:**

**_Local:_**
* **lls** - like ls in linux _(Local on Client machine)_.
* **lcd** - like cd in linux _(Local on Client machine)_.
* **lpwd** - like pwd in linux _(Local on Client machine)_.  

**_Server:_**
* **ls** - like ls in linux _(on Server machine)_.
* **cd** - like cd in linux _(on Server machine)_.
* **pwd** - like pwd in linux _(on Server machine)_.
* **get _(filename)_** - downloading file from actual server directory to actual local directory.
* **put _(local filename) (new name on server machine)_** - uploading file from actual local directory to actual server directory.  

**_Commands can be easily extended on server, execvp is used here._**  

**_Multi-client server using select() (no Threads)._**
