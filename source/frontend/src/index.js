import "./styles.css";

import * as rive from "@rive-app/canvas";

let eventPlaying = "Idle"; 


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
  src: "slideraugmented.riv",
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



function onRiveEventReceived(riveEvent) {
  const eventData = riveEvent.data;
  eventPlaying = eventData.name;
  console.log(eventData.name);

}

riveInstance.on(rive.EventType.RiveEvent, onRiveEventReceived);

riveCanvas.addEventListener(
    "mousemove",
    (event) => {
            const inputs = riveInstance.stateMachineInputs('Loop');

            const rect = event.target.getBoundingClientRect();
            const mouseYRelative = (event.clientY - rect.top) / rect.height;
            const newValue = 100 * (1 - mouseYRelative);
            if(eventPlaying == "InSlider1")
            {
                const posInput = inputs.find(i => i.name === 'Slider 1 Pos');
                posInput.value = newValue;
                reverbUpdated(1 - mouseYRelative);

            }
            else if(eventPlaying == "InSlider2")
            {
                const posInput = inputs.find(i => i.name === 'Slider 2 Pos');
                posInput.value = newValue;
                phaserUpdated(mouseYRelative);
            }
            else if(eventPlaying == "InSlider3")
            {
                
                const posInput = inputs.find(i => i.name === 'Slider 3 Pos');
                posInput.value = newValue;
                gainUpdated(mouseYRelative);

            }
            else 
            {
                
            }


    },
    false
);


// function updateValue(newValue)
// {
//     const inputs = riveInstance.stateMachineInputs('Loop');
//     // const clickInSlider1Input = inputs.find(i => i.name === 'ClickInSlider1');
//     const posInput = inputs.find(i => i.name === 'Slider 1 Pos');

//     // clickInSlider1Input.value = true;
//     // posInput.value = newValue;
//     // clickInSlider1Input.value = false;
// }

// window.updateValue = updateValue;