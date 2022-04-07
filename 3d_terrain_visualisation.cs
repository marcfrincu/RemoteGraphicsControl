// 3D visualisation tool, demonstrates use of Managed DirectX 9.0c and C#.
// Copyright (C) 2005 Igor Stjepanovic
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.Data;
using System.IO;
using Microsoft.DirectX;
using Microsoft.DirectX.Direct3D;
using Microsoft.DirectX.DirectInput;

using RGCDirectX;

namespace Brisbane
{
      public class WinForm : System.Windows.Forms.Form
      {
            private RGC rgc = new RGC();
            private int GRID_WIDTH = 79;     // grid width
            private int GRID_HEIGHT = 88;    // grid height
            private Microsoft.DirectX.Direct3D.Device device = null;  // device object
            private Microsoft.DirectX.DirectInput.Device keyb = null; // keyboard
            private float angleZ = 0f;       // POV Z
            private float angleX = 0f;       // POV X
            private float[,] heightData;     // array storing our height data
            private int[] indices;           // indices array
            private IndexBuffer ib = null; 
            private VertexBuffer vb = null;
            
            private Texture tex = null;
			//Points (Vertices)
			public struct dVertex
			{
				public float x;
				public float y;
				public float z;
			}
	
			//Created Triangles, vv# are the vertex pointers
			public struct dTriangle
			{
				public long vv0;
				public long vv1;
				public long vv2;
			}
            private System.ComponentModel.Container components = null;

            public WinForm(string[] args)
            {
                rgc.rgcSetTaskId(args[0]);
                rgc.rgcReadTaskFile(args[0]);
                rgc.rgcGetAppStartTime();
                
				InitialiseComponent();
				this.SetStyle(ControlStyles.AllPaintingInWmPaint | ControlStyles.Opaque, true);
            }
            
            /// <summary>
 			/// Initialise Device, Vertex Buffer, Index Buffer and load Texture
			/// </summary> 
            public void InitialiseDevice()
            {
            	// define parameters for our Device object
				PresentParameters presentParams = new PresentParameters();
				presentParams.Windowed = true;
				presentParams.SwapEffect = SwapEffect.Discard;
				presentParams.EnableAutoDepthStencil = true;
				presentParams.AutoDepthStencilFormat = DepthFormat.D16;
				// declare the Device object
				device = new Microsoft.DirectX.Direct3D.Device(0, Microsoft.DirectX.Direct3D.DeviceType.Hardware, this, CreateFlags.SoftwareVertexProcessing, presentParams);
				device.RenderState.FillMode = FillMode.Solid;
				device.RenderState.CullMode = Cull.None;                  
				device.DeviceReset += new EventHandler(this.OnDeviceReset); // Hook the device reset event
	            this.OnDeviceReset(device, null);
	            this.Resize += new EventHandler(this.OnResize);

    			// create VertexBuffer to store the points
				vb = new VertexBuffer(typeof(CustomVertex.PositionTextured), GRID_WIDTH * GRID_HEIGHT, device, Usage.Dynamic | Usage.WriteOnly, CustomVertex.PositionTextured.Format, Pool.Default);
				vb.Created += new EventHandler(this.OnVertexBufferCreate);
				OnVertexBufferCreate(vb, null);
				// create IndexBuffer to store indices into vertex data
				ib = new IndexBuffer(typeof(int), (GRID_WIDTH - 1) * (GRID_HEIGHT - 1) * 6, device, Usage.WriteOnly, Pool.Default);
				ib.Created += new EventHandler(this.OnIndexBufferCreate);
				OnIndexBufferCreate(ib, null);
            }
            
            /// <summary>
 			/// populate the indices array used to generate the triangle list
			/// </summary>  
            private void InitialiseIndices()
            {
                  indices = new int[(GRID_WIDTH-1)*(GRID_HEIGHT-1)*6];
                  for (int x = 0; x < GRID_WIDTH - 1; x++)
                  {
                        for (int y = 0; y < GRID_HEIGHT-1; y++)
                        {
                             indices[(x+y*(GRID_WIDTH-1))*6] = (x+1)+(y+1)*GRID_WIDTH;
                             indices[(x+y*(GRID_WIDTH-1))*6+1] = (x+1)+y*GRID_WIDTH;
                             indices[(x+y*(GRID_WIDTH-1))*6+2] = x+y*GRID_WIDTH;
 
                             indices[(x+y*(GRID_WIDTH-1))*6+3] = (x+1)+(y+1)*GRID_WIDTH;
                             indices[(x+y*(GRID_WIDTH-1))*6+4] = x+y*GRID_WIDTH;
                             indices[(x+y*(GRID_WIDTH-1))*6+5] = x+(y+1)*GRID_WIDTH;
                        }
                  }            	
            }            

            /// <summary>
 			/// Initialise the Keyboard device
			/// </summary>             
            public void InitialiseKeyboard()
            {
                  keyb = new Microsoft.DirectX.DirectInput.Device(SystemGuid.Keyboard);
                  keyb.SetCooperativeLevel(this, CooperativeLevelFlags.Background | CooperativeLevelFlags.NonExclusive);
                  keyb.Acquire();
            }
             
            /// <summary>
 			/// Handle camera positioning. This procedure is executed every time 
 			/// the user moves around using arrow keys
			/// </summary>              
            private void CameraPositioning()
            {
                  device.Transform.Projection = Matrix.PerspectiveFovLH((float)Math.PI/4, this.Width/this.Height, 1f, 350f);
                  device.Transform.View = Matrix.LookAtLH(new Vector3(0, -70, -35), new Vector3(0, -5, 0), new Vector3(0, 1, 0));
                  device.RenderState.Lighting = false;
                  device.RenderState.CullMode = Cull.None;
            }
                       
	        /// <summary>
	        /// Called when the form is resized
	        /// </summary>            
	        private void OnDeviceReset(object sender, EventArgs e)
	        {
	        	// use anti-aliasing by applying Texture filters
				if (device.DeviceCaps.TextureFilterCaps.SupportsMinifyAnisotropic)
				{
				    device.SamplerState[0].MinFilter = TextureFilter.Anisotropic;
				}
				else if (device.DeviceCaps.TextureFilterCaps.SupportsMinifyLinear)
				{
				    device.SamplerState[0].MinFilter = TextureFilter.Linear;
				}
				
				// Do the same thing for magnify filter
				if (device.DeviceCaps.TextureFilterCaps.SupportsMagnifyAnisotropic)
				{
				    device.SamplerState[0].MagFilter = TextureFilter.Anisotropic;
				}
				else if (device.DeviceCaps.TextureFilterCaps.SupportsMagnifyLinear)
				{
				    device.SamplerState[0].MagFilter = TextureFilter.Linear;
				}
	        }            
 
            /// <summary>
 			/// Populate the Vertex buffer with points stored in file called m15.txt
			/// </summary>              
            private void OnVertexBufferCreate(object sender, EventArgs e)
            {
                  string line;
				  string[] point;
				  float u, v;
            	  
                  // create texture (you can replace 2001.jpg with other images attached to this project)
                  tex = new Texture(device, new Bitmap("2001.jpg"), Usage.Dynamic, Pool.Default);				  
				  
                  VertexBuffer buffer = (VertexBuffer)sender;
                  CustomVertex.PositionTextured[] verts = new CustomVertex.PositionTextured[GRID_WIDTH * GRID_HEIGHT];
                  
                  System.IO.StreamReader file = new System.IO.StreamReader("m15.txt");
                  for (int x = GRID_WIDTH - 1; x >= 0; x--)
                  {
                        for (int y = GRID_HEIGHT - 1; y >= 0; y--)
                        {
							 line = file.ReadLine();
                        	 point = line.Split(' ');                             
                             u = System.Convert.ToSingle(x) / System.Convert.ToSingle(GRID_WIDTH - 1);
                             v = System.Convert.ToSingle((GRID_HEIGHT - 1) - y) / System.Convert.ToSingle(GRID_HEIGHT - 1);
                             // my city Brisbane is fairly flat, hence I exaturated the Z value by a factor of 5 to make
                             // the relief appear a bit more dramatic :)
                             verts[x+y*GRID_WIDTH] = new CustomVertex.PositionTextured(x, y, -(heightData[x,y]) * 5,  u, v);
                        }
                  }                  
 				  file.Close();
 				  // reset Vertex Buffer
                  buffer.SetData(verts, 0, LockFlags.None);
            }
 
            /// <summary>
 			/// Create the Index Buffer
			/// </summary>            
            private void OnIndexBufferCreate(object sender, EventArgs e)
            {
            	  IndexBuffer buffer = (IndexBuffer)sender;
                  ib.SetData(indices, 0, LockFlags.None);
            }

            /// <summary>
 			/// OnResize fires when the form is resized
			/// </summary>  
            protected  void OnResize(object sender, EventArgs e)
            {
                  if (this.Width < this.Height)
                  	this.Width = this.Height;             
            }            
            
            /// <summary>
 			/// OnPaint fires when the screen needs to be redrawn
			/// </summary>  
            protected override void OnPaint(System.Windows.Forms.PaintEventArgs e)
            {
                  device.Clear(ClearFlags.Target | ClearFlags.ZBuffer, Color.White , 1.0f, 0);
                  //Before scene rendering handle ops
                  rgc.rgcManageOperations();
                  // set the camera position
                  CameraPositioning();
                  // draw the scene	  
                  device.BeginScene();
                  device.SetTexture(0, tex);
                  device.VertexFormat = CustomVertex.PositionTextured.Format;
                  device.SetStreamSource(0, vb, 0);
                  device.Indices = ib;
                  //Adjust scene angles and distances
                  device.Transform.World = Matrix.Translation(-GRID_WIDTH / 2, -GRID_HEIGHT / 2, 0) * rgc.rgcNewAngles();
                  device.DrawIndexedPrimitives(PrimitiveType.TriangleList, 0, 0, GRID_WIDTH * GRID_HEIGHT, 0, indices.Length/3);
                  device.EndScene(); 
                  device.Present();
                  this.Invalidate();
                  ReadKeyboard();
                  //Print the scene to an image file
                  rgc.rgcPrintScene(ref device);
                  //Manage Idle  
                  rgc.rgcManageIdle();
                  // uncomment the following line if you wish to see the scene rotate on its own
                  //angleZ += 0.001f;
            }
 
            /// <summary>
 			/// ReadKeyboard function updates camera position and sets the FillMode
 			/// to WireFrame, Solid and Point modes
			/// </summary>              
            private void ReadKeyboard()
            {
                  KeyboardState keys = keyb.GetCurrentKeyboardState();
                  if (keys[Key.LeftArrow])
                  {
                        angleZ += 0.06f;
                  }
                  if (keys[Key.RightArrow])
                  {
                        angleZ -= 0.06f;
                  }
                  if (keys[Key.UpArrow])
                  {
                        angleX += 0.03f;
                  }
                  if (keys[Key.DownArrow])
                  {
                        angleX -= 0.03f;
                  }                  
                  if (keys[Key.W])
                  {
                  		device.RenderState.FillMode = FillMode.WireFrame;
                  }
                  if (keys[Key.S])
                  {
                  		device.RenderState.FillMode = FillMode.Solid;
                  }
                  if (keys[Key.P])
                  {
                  		device.RenderState.FillMode = FillMode.Point;
                  		device.RenderState.PointSize = 2.0f; // double the point size
                  }
				  if (keys[Key.L])
                  {
                  		device.RenderState.Lighting = true;
                  }                                    
            }

			/// <summary>
 			/// The height data is provided in a series of text files 
 			/// commonly referred to as facets or tiles in the GIS,
 			/// storing heights from top left to bottom right. However,
 			/// the world data is constructed from bottom left
 			/// to top right coordinates, hence the "inverted" Y coords.
			/// </summary> 			
            private void LoadHeightData()
            {
                  string line;
                  string[] point;
				  heightData = new float[GRID_WIDTH, GRID_HEIGHT];
				  float z;
				  
                  System.IO.StreamReader file = new System.IO.StreamReader("m15.txt");
                  for (int y = GRID_HEIGHT - 1; y >= 0; y--)
                  {
                        for (int x = 0; x < GRID_WIDTH; x++)
                        {
                        	line = file.ReadLine();
                        	point = line.Split(' ');
                        	// heightData[GRID_WIDTH-1-y,GRID_HEIGHT-1-x] = System.Convert.ToSingle(point[2]);
                        	// z value has to be divided by 20 to "scale" the height appropriatelly
                        	// remember we've chosen the grid of 79x88 which represents 
                        	// 1580m x 1760m. This is a scale of 20:1. Therefore, a height of
                        	// 20m is only 1 unit in our projected coordinates.
                        	z = System.Convert.ToSingle(point[2]) / 20;
                        	heightData[x, y] = z;
                        }
                  }
                  file.Close();
            }
            
			/// <summary>
 			/// Release resources held by this applications
			/// </summary> 	
            protected override void Dispose (bool disposing)
            {
                  if (disposing)
                  {
                        if (components != null)
                        {
                             components.Dispose();
                        }
                  }
                  base.Dispose(disposing);
            }

			/// <summary>
 			/// Initialise the form object 
			/// </summary> 	            
            private void InitialiseComponent() 
            {
				// 
				// WinForm
				// 
				this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
				this.ClientSize = new System.Drawing.Size(592, 566);
				this.Name = "WinForm";
				this.Text = "Brisbane CBD 3D model";
			}
            
			/// <summary>
 			/// The main function
			/// </summary> 	        
            static void Main(string[] args) 
            {
                  using (WinForm directx_form = new WinForm(args))
                  {
                        directx_form.LoadHeightData();
                        directx_form.InitialiseIndices();
                        directx_form.InitialiseDevice();                        
                        directx_form.InitialiseKeyboard();
                        directx_form.CameraPositioning();
                        directx_form.Show();
                        Application.Run(directx_form);
                  }
            }
      }
}
