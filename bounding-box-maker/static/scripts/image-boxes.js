window.addEventListener('DOMContentLoaded', main);

class rect {
    constructor(beginX, beginY) {
        this.startX = beginX;
        this.endX = beginX;
        this.startY = beginY;
        this.endY = beginY;
    }
    startX = 0;
    startY = 0;
    w = 0;
    h = 0;
};

let canvas = HTMLDivElement;
let image = HTMLImageElement;
let new_box = HTMLDivElement;
let accept_form = HTMLFormElement;
let discard_form = HTMLFormElement;
let data_input = HTMLInputElement;
let accept_btn = HTMLInputElement;
let rects = [];
let drag = false;

function mousedown(e) {
    // if not a left click
    if (e.button != 0) {
        return;
    }

    rects.push(new rect(
        e.pageX - this.offsetLeft,
        e.pageY - this.offsetTop,
    ));
    new_box = document.createElement('div');
    new_box.style.border = "2px solid #fc03a5";
    new_box.style.position = "absolute";
    new_box.style.width = "0px";
    new_box.style.height = "0px";
    new_box.style.left = String(e.pageX - this.offsetLeft) + "px";
    new_box.style.top = String(e.pageY - this.offsetTop) + "px";
    new_box.id = "box" + String(rects.length);
    new_box.classList.add("boxes");
    
    canvas.appendChild(new_box);
    
    drag = true;
}

function mousemove(e) {
    if (drag) {
        let currX = (e.pageX - this.offsetLeft) > image.width ? image.width : (e.pageX - this.offsetLeft);
        let currY = (e.pageY - this.offsetTop) > image.height ? image.height : (e.pageY - this.offsetTop);
        
        // update new position
        let rect = rects.slice(-1)[0]
        rect.w = (currX) - rect.startX;
        rect.h = (currY) - rect.startY;
        let update_box = canvas.lastChild;
        
        if (rect.w <= 0) {
            update_box.style.left = String(e.pageX - this.offsetLeft) +"px";
            update_box.style.width = String(-rect.w) + "px";
        } else {
            update_box.style.width = String(rect.w) + "px";
        }
        
        if (rect.h <= 0) {
            update_box.style.top = String(e.pageY - this.offsetTop) + "px";
            update_box.style.height = String(-rect.h) +"px";
        } else {
            update_box.style.height = String(rect.h) +"px";
        }

        rects[rects.length-1] = rect;        
    }
}
function mouseup(e) {
    drag = false;
}

let undo = () => {
    if (canvas.lastChild) {
        drag = false;
        canvas.removeChild(canvas.lastChild);
        rects.pop();
    }
}

let setup_canvas = () => {
    canvas = document.getElementById("canvas");
    canvas.style.marginTop = String(-image.clientHeight) + "px";
    canvas.style.height = String(image.clientHeight) + "px";
    canvas.addEventListener('mousedown', mousedown);
    canvas.addEventListener('mousemove', mousemove);
    canvas.addEventListener('mouseup', mouseup);
    document.addEventListener('keydown', e => {
        if (e.ctrlKey && e.key === 'z') {
            undo();
        }
    });
};

function main() {
    image = document.getElementById("image");
    accept_form = document.getElementById("accept");

    if (image.complete) {
        setup_canvas();
    } else {
        image.addEventListener('load', setup_canvas);
    }

    accept_btn = document.getElementById("accept-btn");

    accept_btn.addEventListener('click', (e) => {
        e.preventDefault();
        if (rects.length !== 0) {
            data_input = document.createElement("input");
            data_input.type = "hidden";
            data_input.name = "data";

            data_input.value = "";
            // No. of boxes
            data_input.value += String(rects.length) + " ";
            // Image scaled dimensions in pixels
            data_input.value += String(image.width) + " " + String(image.height);
            
            for (let i = 0; i < rects.length; i++) {
                let startX = rects[i].startX;
                let startY = rects[i].startY;
                let w = rects[i].w;
                let h = rects[i].h;

                // Clamp box coords within image
                w = startX + w > image.width ? image.width : w;
                w = startX + w < 0 ? 0 : w;
                h = startY + h > image.height ? image.height : h;
                h = startY + h < 0 ? 0 : h;

                // Center, (x, y)
                data_input.value += " " + String(w/2 + startX) + " " + String(h/2 + startY);
                // Absolute value of width and height
                data_input.value += " " + String(w >= 0 ? w : -w) + " " +  String(h >= 0 ? h : -h);
            }

            accept_form.appendChild(data_input);
        }
        accept_form.submit();            
    });
}




