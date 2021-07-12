from django.shortcuts import render
from .models import Planta, Especie
from django.utils import timezone
from django.http import HttpResponse
from django.views.decorators.csrf import csrf_exempt

@csrf_exempt
def visaoPlanta(request):
    template_name = 'main.html'
    planta = Planta.objects.filter()[0]
    if request.method == 'POST':
        espData = request.POST['especie']
        lumData = request.POST['luminosidade']
        umiData = request.POST['umidade']
        print('a', espData, 'b', lumData,'c', umiData)

        if espData!='0':
            planta.ilu_manual = None
            planta.umi_manual = None
            planta.especie = Especie.objects.filter(id=int(espData))[0]
            planta.save()
        else:
            planta.ilu_manual = int(lumData)
            planta.umi_manual = int(umiData)
            planta.especie = None
            planta.save()
    especies = Especie.objects.filter()
    (ilu, umi) = planta.dados()
    ts = []
    for t in planta.ts_dados:
        ts.append(t.timestamp()*1000)
    context = {'ilu_ideal':ilu, "umi_ideal":umi, "especies":especies, "ilu_dados":planta.ilu_dados, "umi_dados":planta.umi_dados, "ts_dados":ts}
    return render(request, template_name, context)

@csrf_exempt
def dadosPlantas(request):
        try:
            data = request.body.decode('utf-8').split(',')
            print(data)
            planta = Planta.objects.filter(serial=data[0]).first()
            (ilu, umi) = planta.dados()
            respData = str(ilu)+','+str(umi)
            planta.ilu_dados.append(data[1])
            planta.umi_dados.append(data[2])
            planta.ts_dados.append(timezone.now())
            planta.save()
            return HttpResponse(respData)
        except Exception as e:
            print(e)
            return HttpResponse(status=400)