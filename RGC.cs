/*
  Remote Graphic Controller API for managed DirectX
  Created by Marc Frincu 
  Email: marc@ieat.ro
  June 20th 2007
*/

using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Windows.Forms;
using Microsoft.DirectX;
using Microsoft.DirectX.Direct3D;

namespace RGCDirectX
{
    /// <summary>
    /// This class corresponds to the 3rd layer API, and is responsable for handling the incoming
    /// requests from the client. It offers transparent methods for handling different actions
    /// such as scene and camera manipulation, mouse and keyboard input, capturing scene events.
    /// </summary>
    /// <remarks>The functionality of some methods can be adapted to suit the requirements of particular applications</remarks>
    class RGC
    {
        //Maximum number of operations that can be sent from the client
        private static int OPERNUMBER = 11;
        private Operations[] operations = new Operations[OPERNUMBER];
        //True if there is ok to take a screenshot. Usually true when receiving a request
        private bool okShot = false;
        //The ID of the last generated image
        private int lastId = 0;
        //The time when the last task was received
        private DateTime timeLastTask;
        //The time since the last check for a new task was made
        private DateTime lastTime;
        //The ID of the task. It is received from the client
        private long taskId = 0;
        //Angles & distances used for scene and camera manipulation
        private float rgcAngleZ = 0f;
        private float rgcAngleX = 0f;
        private float xrotate = 0, yrotate = 0, zrotate = 0, xtranslate = 0, ytranslate = 0, ztranslate = 0, zoom = 0;
        //Mouse (x,y) coordinates
        private int rgcMouseDownX = -1;
        private int rgcMouseDownY = -1;
        //The unicode of the key pressed on the client side
        private int rgcKeyUnicode = -1;

        //These 2 properties can be used in the main application to handle mouse clicks
        public int RGCMouseX
        {
            get
            {
                return rgcMouseDownX;
            }
        }
        public int RGCMouseY
        {
            get
            {
                return rgcMouseDownY;
            }
        }

        #region Task
        /// <summary>
        /// Holds the information regarding the operations sent by the client
        /// </summary>
        private struct Operations
        {
            public string name;
            public float value;
        }

        /// <summary>
        /// Method responsable for reading the task file. It also manages eventual errors
        /// </summary>
        /// <remarks>Will be improved by reading from an XML file</remarks>
        /// <param name="filename">The task file name</param>
        public void rgcReadTaskFile(string filename)
        {
            StreamReader file;
            try
            {
                file = new StreamReader(filename);
                operations = new Operations[OPERNUMBER];
                String line = file.ReadLine();
                if (line == null)
                {
                    okShot = false;
                    file.Close();
                    return;
                }
                int i = 0;
                while (line != null)
                {
                    timeLastTask = rgcGetTimeLastTask();
                    okShot = true;
                    string[] items = line.Split('\t');
                    if (items.Length == 2)
                    {
                        operations[i].name = items[0];
                        operations[i].value = Convert.ToSingle(items[1]);
                        i++;
                    }
                    line = file.ReadLine();
                }
                file.Close();
                FileStream fs = new FileStream(filename, FileMode.Truncate);
                fs.Close();
            }
            catch (Exception)
            {
                //TODO: Write log
            }
        }

        /// <summary>
        /// Sets the taskId
        /// </summary>
        /// <param name="newTaskId">The task ID taken from the first argument passed to the application</param>
        public void rgcSetTaskId(string newTaskId)
        {
            try
            {
                taskId = Convert.ToInt64(newTaskId);
            }
            catch (Exception ex)
            {
                //TODO
                MessageBox.Show(newTaskId + "\n" + ex.Message);
            }
        }

        #endregion

        #region Time Related
        /// <summary>
        /// Gets the current time
        /// </summary>
        /// <returns>The date & time</returns>
        public DateTime rgcGetTime()
        {
            return DateTime.Now;
        }

        /// <summary>
        /// Gets the time of the last task
        /// </summary>
        /// <returns>The date & time</returns>
        public  DateTime rgcGetTimeLastTask()
        {
            return rgcGetTime();
        }

        /// <summary>
        /// Sets the last access time to be the application start time. It is usually called only once
        /// when the application starts
        /// </summary>
        public void rgcGetAppStartTime()
        {
            lastTime = rgcGetTime();
        }

        #endregion

        #region Manage Idle, Operations, Mouse, Keys
        /// <summary>
        /// Manages the events that happen between scene displays: tries to read the task file or exits the application if
        /// it has been a long time since the last task was sent
        /// </summary>
        public  void rgcManageIdle()
        {
            TimeSpan ts = rgcGetTime() - lastTime;
            if (ts.TotalMilliseconds > 3000)
            {
                rgcReadTaskFile(Convert.ToString(taskId));
                lastTime = rgcGetTime();
            }
            ts = rgcGetTimeLastTask() - timeLastTask;
            if (ts.TotalMilliseconds > 120000)
            {
                Application.Exit();
            }
        }

        /// <summary>
        /// Manages the operations. It is called before the scene is being rendered
        /// </summary>
        public  void rgcManageOperations()
        {
            rgcMouseDownX = -1;
            rgcMouseDownY = -1;
            rgcKeyUnicode = -1;
            for (int i = 0; i < 11; i++)
            {
                switch (operations[i].name)
                {
                    case "xrotate":
                        if (okShot == true)
                        {
                            xrotate += operations[i].value;
                        }
                        break;
                    case "yrotate":
                        if (okShot == true)
                        {
                            yrotate += operations[i].value;
                        }
                        break;
                    case "zrotate":
                        if (okShot == true)
                        {
                            zrotate += operations[i].value;
                        }
                        break;
                    case "xtranslate":
                        if (okShot == true)
                        {
                            xtranslate += operations[i].value;
                        }
                        break;
                    case "ytranslate":
                        if (okShot == true)
                        {
                            ytranslate += operations[i].value;
                        }
                        break;
                    case "ztranslate":
                        if (okShot == true)
                        {
                            ztranslate += operations[i].value;
                        }
                        break;
                    case "zoom":
                        zoom = operations[i].value;
                        break;
                    case "key":
                        if ((okShot == true) && ((int)operations[i].value != -1))
                        {
                            rgcManageKeys((int)operations[i].value);
                            rgcKeyUnicode = (int)operations[i].value;
                            operations[i].value = -1;
                        }
                        break;
                    case "mousex":
                        if (okShot == true)
                        {
                            rgcMouseDownX = (int)operations[i].value;
                        }
                        break;
                    case "mousey":
                        if (okShot == true)
                        {
                            rgcMouseDownY = (int)operations[i].value;
                        }
                        break;
                    case "quit":
                        if (operations[i].value == 1)
                        {
                            Application.Exit();
                        }
                        break;
                }
            }
        }

        /// <summary>
        /// Handles the operations with respect to a certain key
        /// </summary>
        /// <param name="unicode"></param>
        public void rgcManageKeys(int unicode)
        {
            /*key unicodes*/
            switch (unicode)
            {
                /*left key*/
                case 37:
                    rgcAngleZ += .06f; rgcOrientMe(rgcAngleZ); break;
                /*right key*/
                case 39:
                    rgcAngleZ -= .06f; rgcOrientMe(rgcAngleZ); break;
                /*up key*/
                case 38:
                    rgcAngleX += .03f; rgcMoveMeFlat(rgcAngleX); break;
                /*down key*/
                case 40:
                    rgcAngleX -= .03f; rgcMoveMeFlat(rgcAngleX); break;
                /*Other keys can be added here*/
            }
        }

        /// <summary>
        /// Computes the new angle matrix
        /// </summary>
        /// <remarks>This method can be modified to fit the necesites of the application</remarks>
        /// <returns>The resulted matrix</returns>
        public  Matrix rgcNewAngles()
        {
            return Matrix.RotationZ(rgcAngleZ) * Matrix.RotationX(rgcAngleX);
        }
        /// <summary>
        /// Orients the camera
        /// </summary>
        /// <param name="angle">The angle</param>
        private void rgcOrientMe(float angle)
        {
            //TODO
        }

        /// <summary>
        /// Moves the camera
        /// </summary>
        /// <param name="distance">The distance</param>
        private void rgcMoveMeFlat(float distance)
        {
            //TODO
        }

        #endregion

        #region PrintScene
        /// <summary>
        /// Method called after the scene has been rendered. It manages the taking of the screen shot
        /// </summary>
        /// <param name="device"></param>
        public  void rgcPrintScene(ref Device device)
        {
            if (okShot == true)
            {
                string filename = "dir"+ taskId+"/test" + lastId + ".bmp" /*.jpg*/;
                rgcTakeScreenshotBMP(ref device, filename);
                okShot = false;
                lastId++;
            }
        }

        /// <summary>
        /// Takes a screen shot and saves it to a BMP format
        /// </summary>
        /// <param name="device">A reference to the Direct3D device</param>
        /// <param name="filename">The BMP filename where the screenshot is to be saved</param>
        public  void rgcTakeScreenshotBMP(ref Microsoft.DirectX.Direct3D.Device device, string filename)
        {
                Surface backbuffer = device.GetBackBuffer(0, 0, BackBufferType.Mono);
                SurfaceLoader.Save(filename, ImageFileFormat.Bmp, backbuffer);
                backbuffer.Dispose();
        }

        /// <summary>
        /// Takes a screen shot and saves it to a JPG format
        /// </summary>
        /// <param name="device">A reference to the Direct3D device</param>
        /// <param name="filename">The JPG filename where the screenshot is to be saved</param>
        public void rgcTakeScreenshotJPG(Microsoft.DirectX.Direct3D.Device device, string filename)
        {
            Surface backbuffer = device.GetBackBuffer(0, 0, BackBufferType.Mono);
            SurfaceLoader.Save(filename, ImageFileFormat.Jpg, backbuffer);
            backbuffer.Dispose();
        }

        #endregion
    }
}
