package {
    
	import flash.events.MouseEvent;
    import PaperBase;
    import org.papervision3d.objects.DisplayObject3D;
    import org.papervision3d.objects.parsers.Collada;
	import org.papervision3d.core.math.Matrix3D;
	import org.papervision3d.objects.primitives.Plane;
	import org.papervision3d.materials.WireframeMaterial;
	import org.papervision3d.materials.ColorMaterial;
	import org.papervision3d.materials.utils.MaterialsList;
    
    public class TestModel extends PaperBase {
       
       public var testModel:DisplayObject3D;
       private var xDist:Number;
	   private var yDist:Number;
	   private var locked:Boolean;
	   // var cameraMatrix:Matrix3D;
	   private var wireframeMaterial:WireframeMaterial;
	   private var xyPlane:Plane;
	   private var xzPlane:Plane;
       private var yzPlane:Plane;
       
	   public function TestModel()
	   {
		    init();
			xDist = mouseX - stage.stageWidth * 0.5;
			yDist = mouseY - stage.stageHeight * 0.5;
			locked = true;
			//cameraMatrix = new Matrix3D(new Array(0,0,1000));							
       }
       
       override protected function init3d():void 
	   {
      		// LOAD MODEL HERE
			testModel = new Collada("checkeredSphereJPG.DAE");
			//testModel = new Collada("checkeredSpherePNG.DAE");
			//testModel = new Collada("cowSphere.DAE");
         	//testModel.moveLeft(550);
			//testModel.moveUp(310);
         	testModel.scale = 2.5;
		 	//testModel.pitch(-10);
			
			wireframeMaterial = new WireframeMaterial(0x333333);
			wireframeMaterial.doubleSided = true;
			xyPlane = new Plane(wireframeMaterial, 600, 600, 8, 8); 
			xzPlane = new Plane(wireframeMaterial, 600, 600, 8, 8); 
			yzPlane = new Plane(wireframeMaterial, 600, 600, 8, 8); 
         	
			default_scene.addChild(testModel);
			//default_scene.addChild(xyPlane);
			//default_scene.addChild(xzPlane);
			//default_scene.addChild(yzPlane);
			
			xzPlane.rotationX = 90;
			yzPlane.rotationY = 90;
			
			stage.addEventListener(MouseEvent.MOUSE_DOWN, stageMouseDown);
			stage.addEventListener(MouseEvent.MOUSE_UP, stageMouseUp);
       }
       
       override protected function processFrame():void 
	   {
        	default_camera.y = (mouseY * 4)-800;
			default_camera.x = (mouseX * 4)-800;
			//testModel.yaw(5);
			if(!locked)
			{
				// Crazy Cow With Mouse Click
				/*xDist = mouseX - stage.stageWidth * 0.5;
				yDist = mouseY - stage.stageHeight * 0.5;
				testModel.rotationY -= xDist * 0.05;
				testModel.rotationX -= -yDist * 0.05;*/
			}	
			//default_camera.x = -(((mouseX - (stage.width / 2)) / stage.width) * 1600);
			//default_camera.y = -(((mouseY - (stage.height / 2)) / stage.height) * 1600);			
			//testModel.rotationY = -((mouseX / stage.width) * 360) 
       }       
	   
	   private function stageMouseDown(me:MouseEvent):void
	   {
		   locked = false;
	   }
	   
	   private function stageMouseUp(me:MouseEvent):void
	   {
		   locked = true;
	   }
    }    
} 