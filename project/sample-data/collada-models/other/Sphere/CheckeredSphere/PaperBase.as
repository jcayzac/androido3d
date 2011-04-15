/**
* ...
* @author Luke Mitchell
* @version 1.0.0
*/

package  {
	// These lines make differant 'pieces' available in your code.
	import flash.display.Sprite; // To extend this class
	import flash.events.Event; // To work out when a frame is entered.
	import flash.media.Camera;
	import org.papervision3d.core.proto.CameraObject3D;
	import org.papervision3d.core.utils.InteractiveSceneManager;
	
	import org.papervision3d.view.Viewport3D; // We need a viewport
	import org.papervision3d.cameras.*; // Import all types of camera
	import org.papervision3d.scenes.Scene3D; // We'll need at least one scene
	import org.papervision3d.render.BasicRenderEngine; // And we need a renderer
	//import org.papervision3d.core.math.Matrix3D;
	
	public class PaperBase extends Sprite { //Must be "extends Sprite"
		
		//public var cameraMatrix:Matrix3D;
		public var viewport:Viewport3D; // The Viewport
		public var renderer:BasicRenderEngine; // Rendering engine
		// -- Scenes -- //
		public var default_scene:Scene3D; // A Scene
		// -- Cameras --//
		public var default_camera:Camera3D; // A Camera
		public var current_camera:CameraObject3D; // A Camera
		
		public function init(vpWidth:Number = 550, vpHeight:Number = 400):void {
			initPapervision(vpWidth, vpHeight); // Initialise papervision
			init3d(); // Initialise the 3d stuff..
			init2d(); // Initialise the interface..
			initEvents(); // Set up any event listeners..			
		}
		
		protected function initPapervision(vpWidth:Number, vpHeight:Number):void {
			// Here is where we initialise everything we need to
			// render a papervision scene.
			viewport = new Viewport3D(vpWidth, vpHeight, false, true);
			// The viewport is the object added to the flash scene.
			// You 'look at' the papervision scene through the viewport
			// window, which is placed on the flash stage.
			addChild(viewport); // Add the viewport to the stage.
			// Initialise the rendering engine.
			renderer = new BasicRenderEngine();
			// -- Initialise the Scenes -- //
			default_scene = new Scene3D();
			// -- Initialise the Cameras -- //
			default_camera = new Camera3D(); // The argument passed to the camera
			// is the object that it should look at. I've passed the scene object
			// so that the camera is always pointing at the centre of the scene.
			current_camera = default_camera;
			//cameraMatrix = new Matrix3D(new Array(0,0,0));
		}
		
		protected function init3d():void {
			// This function should hold all of the stages needed
			// to initialise everything used for papervision.
			// Models, materials, cameras etc.			
		}
		
		protected function init2d():void {
			// This function should create all of the 2d items
			// that will be overlayed on your papervision project.
			// User interfaces, Heads up displays etc.
		}
		
		protected function initEvents():void {
			// This function makes the onFrame function get called for
			// every frame.
			addEventListener(Event.ENTER_FRAME, onEnterFrame);
			// This line of code makes the onEnterFrame function get
			// called when every frame is entered.
		}
		
		protected function processFrame():void {
			// Process any movement or animation here.
		}
		
		protected function onEnterFrame( ThisEvent:Event ):void {
			//We need to render the scene and update anything here.
			processFrame();
			//current_camera.rotationY += 10;
			renderer.renderScene(default_scene, current_camera, viewport);
		}
		
	}
	
}