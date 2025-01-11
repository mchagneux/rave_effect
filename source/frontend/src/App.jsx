import { useState } from 'react'
import reactLogo from './assets/react.svg'
import viteLogo from '/vite.svg'
import './App.css'
import './styles.css'


import { useRive, Layout, Fit, Alignment } from "@rive-app/react-canvas";

export const RiveDemo = () => {
  const { RiveComponent } = useRive({
    // Load a local riv `clean_the_car.riv` or upload your own!
    src: "slider.riv",
    // Be sure to specify the correct state machine (or animation) name
    stateMachines: "Loop",
    // This is optional.Provides additional layout control.
    layout: new Layout({
      fit: Fit.Contain, // Change to: rive.Fit.Contain, or Cover
      alignment: Alignment.Center,
    }),
    autoplay: true,
  });

  return <RiveComponent />;
};


function App() {
  const [count, setCount] = useState(0)

  return (
    <>
      <div className="RiveContainer">
      <RiveDemo />
      </div>

    </>
  )
}

export default App
