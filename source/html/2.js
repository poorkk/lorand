function myFunction2()
{
    var xhr = new XMLHttpRequest()
    xhr.open("POST", "www.baidu.com", true);

    xhr.onload = function {
        if (this.status == 200) {
            document.write(this.response);
        } else {
            document.write("<p>script 其实就是Include</p>");
        }
    }

    document.getElementById("demo2").innerHTML="我的第一个 JavaScript 函数";
    xhr.send();
}