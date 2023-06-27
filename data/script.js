

window.addEventListener("load", () => {
    getAlldata();
    setInterval(getAlldata, 2000);
})


function getAlldata(){
    fetch("http://192.168.1.180/getAllData", {
        method: "GET",
        headers: {
            "Content-Type": "text/plain"
        }
    })
    .then(function (response){
        if(response.ok){
            return response.text();
        }
        else {
            throw new Error("Error: " + response.status);
        }
    })
    .then(function (responseData){
        var jsonResponse = JSON.parse(responseData);

        var keys = jsonResponse.keys;
        var values = jsonResponse.values;


        for(var i = 0; i <= keys.length; ++i) {

            if(document.getElementById(keys[i]) == null) continue;

            var color;
            if(values[i] == true) {
                color = "#ffb703";
            } else {
                color = "#8d99ae";
            }
            document.getElementById(keys[i]).style.backgroundColor = color;
        }
    })
    .catch(function (error){
        console.log(error);
    });
}


function sendHTTPRequest(key){

    var data = {
        "key": key
    }

    var params = new URLSearchParams();
    params.append("data", JSON.stringify(data));


    fetch("http://192.168.1.180/data", {
        method: "POST",
        headers: {
            "Content-Type": "application/x-www-form-urlencoded"
        },
        body: params
    })
    .then(function (response){
        if (response.ok){
            return response.text();
        }
        else {
            throw new Error("Error: " + response.status);
        }
    })
    .then(function (responseData){
        console.log(responseData);

        //aktualisieren
        getAlldata();
    })
    .catch(function (error){
        console.log(error);
    });
}


function allOff(){

    fetch("http://192.168.1.180/allOff", {
        method: "POST",
        headers: {
            "Content-Type": "text/plain"
        },
        body: "Mach aus"
    })
    .then(function (response){
        if (response.ok){
            return response.text();
        }
        else {
            throw new Error("Error: " + response.status);
        }
    })
    .then(function (responseData){
        console.log(responseData);
        //aktualisieren
        getAlldata();
    })
    .catch(function (error){
        console.log(error);
    });
}