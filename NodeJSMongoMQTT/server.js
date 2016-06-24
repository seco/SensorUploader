/**
 * This NodeJS application listens to MQTT messages and records them to MongoDB
 * @author  Dennis de Greef <github@link0.net>, Daniil Bystrukhin <dannsk@mail.ru>
 * @license MIT
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
 * OR OTHER DEALINGS IN THE SOFTWARE.
 */

require('./utils');
var config   = require('./config');

if (!config.debug) {
    console.log = function (){};
    console.info = function (){};
    console.error = function (){};
}


var service  = new (require('./service'))(config);

service.start()
