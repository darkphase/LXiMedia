function version()              { return "0.3.0"; }
function targetAudience()       { return "NL"; }
function category()             { return "Radio"; }

function icon(id)
{ 
  return radio4_png;
}

function listItems(path)
{
  if (path == "/")
    return "stream|Play live stream|Audio";
  else
    return "";
}

function streamLocation(id)
{
  if (id == "stream")
    return "http://shoutcast2.omroep.nl:8106/";
  else
    return "";
}

var radio4_png = "iVBORw0KGgoAAAANSUhEUgAAAIAAAACACAMAAAD04JH5AAAAAXNSR0IArs4c6QAAAGBQTFRFHx4dODY2VFFQn1ikmFydkWCXkWORi2mNfnJ+r3GyloqXrYGwooaly6DNtqm2xqjIvMHDxMPB4MHh3Mve2dbb9Nn04eHe7t3w6ejs/Oz88PTw/vT9//v5/fv/+f7z/f/8Mo2s6gAAAAlwSFlzAAALEwAACxMBAJqcGAAAAAd0SU1FB9sHHxMYMlrbCWUAAABBdEVYdENvbW1lbnQAQ1JFQVRPUjogZ2QtanBlZyB2MS4wICh1c2luZyBJSkcgSlBFRyB2NjIpLCBxdWFsaXR5ID0gOTAKsEVYkwAACRhJREFUeNrtmotyo7gShn0DbK4bYYJlqcP7v+X23y2BcDw78W5ypuoUTCqxwdCf+vKrJc9u+sPHbgPYADaADWAD2AA2gA1gA9gANoANYAPYADaA/z8A+s37/40HiPDzp0JA8/FnAIJpcu4rDD+QA8SG+7Zse/cVL/wEANn2XJbnczu6bwSgZ2n97Onk2D7btn1ZjtbRb6ph93X7OhqKP0m8KaXD+HvnnHUgcHoTPbGvd+1eyi3+42keTZLr0dWceWIfp10gWO6dEtR45+sAfn4MkQ8EeKGn7FjXbTBHrq9rIUBiTtPHpB/7+EiYXgII9vk53qttP60GRm7sCvb/7KG+LkeJkfrJzVnp44uvA/gYzXA7ef+QjNH/zgUPIArsg3ATWTsMg5UyDeb9S2UYHgoPfi5wnHRubM/X+dqdIoHjy3ff1xWOulMIDcmrOsBP9BLC1DVz+Y3duaV11jg31OUNfLbOTnxcsqyozQAn/QsANmJx44Psx0t10a+z3AtWj1tsB/vH4/F0yrK8MlEhXgNwI7NPSQT8XIcy/t4uabkUKM6REeNHIWCGvLMqDl8GkPKzdV7zeNhaLzIbylDj30n9+3WZa+54Gi4wnQmBvBKCFwC03PuCb3QTjXVRD5zswRoAuP57mA+1IQN/f7uHu8cKprOqOp0WAvcCgA7ImfyUMYDr8rzoxkTn2T7X/1KnEpu3Qyl07LlOAtAM1WkhqIbXPCCBzBaAWtMINcH1X9az/sjBQ293+zcBuFmDBMwauwLIOAiveEABjicA8HgheMv4o/4v2jSV+93hDhKC4yTozi4AcMEYAPw8q6STzKT/koy+RwC2iCRUVebhD9BfmrWZ/9zv5W63L+UxboRZDrlTgIDAAH0AIBFPvgynfkxOXgXZ9uG6xUUnIaA7TrAiIbyerwxN1rC6TQuAn9j+bv8XTjlRIORuzMVTqMbcCMB9cr3hzqE3TdPTxweNpuuarjOjAEBjrTFd03RmMHmWtzIo01uRRG9NwxV2aYydZzj+3e4Z4CA+sU3G4tfxxycaquNyRAAurpqVqav54YXzzlQZjpwrHl7m2/omz3OcqprTkT0gZVgYh2s8+2WqLKjCCPAG+7tWHNDlWvRQXluFwScA+Ag7tipg/2wtDxJiCYLc8G139hveCNUFtWR5nudPNxbKWAmZ/FIC2L8dYH//zsnFCZhJ/kk2WdGBkAWnLACISmN4DTeyHLAsqzpjeNRZPvAnQgpxFKr8NANk+EuuAXs3DOw2FLZIi6cW9nclkqfn8XCGhBJ1dZYkgVYBggQXdpJmcGmBenEDns0ucKwhp8ogCwfcfuJoUs/IAEB9VzK7stQiP0WebxKAw10m4wxRi31IBDgmAD6EYNS8J9v3VnJXM14S92S0rxoLFaIA4PE8hvQfnvP0Ik9En6IOeCMeRQWDcov4xtbzbABeFSIvAF3oXL3Wox3HBm5xYquKnWUXAAw8wWXJQJVUA4NXElSKGXBGfYp9yVupTbJVokO5DCsCmNjsogpr9C7waate7iKZSjEBQOA4wJ2dtMcUYHjgTQJwddG+Fo6WZDU7gJPW6lyAKAcAmfErrQCEmwFmOuludDJiAHkqvJMZCAwKstOQ+eksGuTE/yf1NQU5tclsWBidjknTzDgf5k24vOrqppIQKJ2Krog6ZlEnAOIQREIavKljcAD4g4pwXxdZkAgFkH4iy6IIqAMSAJltIBs8a44wcgGADd7GGCOADyFwAz5stb0jFrwc4s41APvX8lwUuTizaEMSkqvVt8xf9PPChLqY58gSTisuZ9afXobnBnjEStdDrtIk9PA957BlAMl80vzKRx7ElWfBG9zNiTxyG27Ris8t3TAOPKMdj3kszd30QXMSCgBLnHhLfOFEvFgSnfdwOYQIADJ0TRDoAheYyAUSUgDisk1mOR09OgeZ0wY8BWMKAD4A4B3Pl3nGqsOVCG1jY1x68KPhkfCpoIRIf3ByDHCNZ7LRVJrJArC/3ubj/Z1/BQ94O9oRM5qIZlgds+TAhJH2QqT7xFMbK3EmPQxmU0w3LNSV5JToOoeA/7JgdgjzBdd4/tC0YhnYH9ZHG9dTtrlcqiqXhjL0q5KERa6iwJow1HmYDHUSIbR7AiOWeMISgBwTLDvXymSodzRhLX7ePR7nuFRDPPkRvFaa2zcA9NLjqljbsSmQuEVnmkIyFQvOCmeKxgxVgUlaumLtgaxRYiw1wv7BX58AyhmAyxDtrE0AZBXNDZabtH8jO3BDgpgPPCtog2PRo3TDyIndo0lASwb7E5ojbsiqnNlsUJIgxSuAefeoxtCwbxKQYkumtcwryOnmkkMaLOl65bAuNmqS3vGh1pm61gn3QxTv7RGgnXvLjj1n4+p5WZr5dK1LT7b5Pp2Jr+4yh3EcZBtgCnsH7do+ZCHuF3DqUmpfekKsu2jp5+Nz1kvzf9jumjdjIsKt3Cf29+3SKzq3Gv6/3aZbw8mTr+dyjM/m4bSRYH8or+k61T26cUefFtpP9lzXe1zLsjO5+VqWg6TSnQH89F4etC+6IbFDxki74tdbLjvdaYkT9tNUeCTDWx+XM/O5MWYiOwjifH0rz4c9x5+93rY3/+g6moVIwk1E66XV2r5PtoQAGzepkuU49sfC+hAX8UzW4atsHJW9+1UK7aKHlx2FZV+SZiv6agaAAxIPqFroCt09eituHCYZFFKaAoBfbTXGzVCal4ayOYmObp0NFJ0Ygon1UlmOzj0AwL6Newk0JaGj2QMPAPpmfWJZwv5q6RwIUmeTbly3bp37a4D/+M0LLdfQs0EPkiKB/+vWLRu1ftlt+c7t+pCJXhWpXxSJxP4cldCZfP/3BX7eV3AcBcnEdOM28QD9zBcWqc/DbpUkEY/f2FWt/jCAiq3ogca/7GXmjTtnz1LpewECAecBVndx4+jnvzNKbEixjiU3VF19/qR/9DMANG9Yh7kQwa/rtp93MnGdfg7gyaYm95bS+PjfCsk3AlASBJ11kwTw9AuM7/eAD2WXfDHkf9Vw/FQI/OMXXXEWfPL13e6bgx9VGC3J8u1U6If80ntu/4NiA9gANoANYAPYADaADWAD2AA2gA1gA9gANoD5+BswOQU9hRP/QQAAAABJRU5ErkJggg==";
