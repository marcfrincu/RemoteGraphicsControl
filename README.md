# RemoteGraphicsControl
A simple way to remotely control your OpenGL/DirectX application through AJAX, XML, and PHP and visualize the results on your HTML page. This project was part of *my first ever published research paper in computer science as single author back in 2007*. You can find the full paper on [Remote Control for Graphic Applications](https://ieeexplore.ieee.org/document/4438114). It was followed by a generalized solution [On Designing an Asynchronous and Dynamic Platform for Solving Single Task Requests of Remote Applications](https://ieeexplore.ieee.org/abstract/document/4591338).

## Code

- **ajax.js** contains the asynchronous calls to the PHP scripts used to enable communication from the browser to the server running the 3D application;
- **requestHandler.php** retrieves the request from the client and writes a timestamped file that is later read by the *rgc* API;
- **resultHandler.php** returns the updated scene (single image/frame);
- **index.html** the HTML client used to send commands and to display the result.

## Disclaimer
The code is distributed as is without any guarantees. This is a version I found on one of my old CDs and is incomplete as the *rgc* API is missing. The paper I published provides an overview of the API.
