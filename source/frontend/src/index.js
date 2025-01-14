import "./styles.css";

import * as rive from "@rive-app/canvas";


let shouldUpdate = false; 


const layout = new rive.Layout({
  fit: rive.Fit.FitWidth, // Change to: rive.Fit.Contain, or Cover
  alignment: rive.Alignment.Center,
});

// ---------------------------------
// HTML Canvas element to render to
const riveCanvas = document.getElementById("rive-canvas");


// ---------------------------------
// Re-adjust the rendering surface if the window resizes
window.addEventListener(
  "resize",
  () => {
    riveInstance.resizeDrawingSurfaceToCanvas();
  },
  false
);

// NOTE: This function is not called in this example.
function cleanUpRive() {
  riveInstance.cleanup();
}


// --[EXAMPLE FROM HERE]
const riveInstance = new rive.Rive({
  // Load a local riv `clean_the_car.riv` or upload your own!
  artbord: "Slider",
  src: "slider.riv",
  // Be sure to specify the correct state machine (or animation) name
  stateMachines: "Loop", // Name of the State Machine to play
  canvas: riveCanvas,
  // artboard: "Artboard" // Optionally provide the artboard to display
  layout: layout, // This is optional. Provides additional layout control.
  autoplay: true,
  onLoad: () => {
    riveInstance.resizeDrawingSurfaceToCanvas();
    riveInstance.enableFPSCounter();
  }
});

riveCanvas.addEventListener("mousedown", () => { shouldUpdate = true;})
riveCanvas.addEventListener("mouseup", () => { shouldUpdate = false;})
riveCanvas.addEventListener("mouseenter", () => { shouldUpdate = false;})

riveCanvas.addEventListener(
    "mousemove",
    (event) => {
        if(shouldUpdate){        
            const inputs = riveInstance.stateMachineInputs('Loop');
            const posInput = inputs.find(i => i.name === 'Pos');
            const rect = event.target.getBoundingClientRect();
            const mouseXRelative = (event.clientX - rect.left) / rect.width;
            const mouseYRelative = (event.clientY - rect.top) / rect.height;
            console.log(`Relative Mouse Position: (${mouseXRelative.toFixed(2)}, ${mouseYRelative.toFixed(2)})`);     
            posInput.value = 100 * (1 - mouseYRelative);
            valueUpdated(1 - mouseYRelative);
        }
    },
    false
);


function updateValue(newValue)
{
    const inputs = riveInstance.stateMachineInputs('Loop');
    const posInput = inputs.find(i => i.name === 'Pos');
    console.log(newValue);
    posInput.value = newValue;

}

window.updateValue = updateValue;