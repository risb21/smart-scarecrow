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
    endX = 0;
    endY = 0;
};

let canvas = HTMLDivElement;
let image = HTMLImageElement;
let new_box = HTMLDivElement;
let rects = [];
let drag = false;

function mousedown(e) {
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
        canvas.removeChild(canvas.lastChild);
        rects.pop();
    }
}

function main() {
    image = document.getElementById("image");
    canvas = document.getElementById("canvas");
    image.addEventListener('load', () => {
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
    });
    



}




