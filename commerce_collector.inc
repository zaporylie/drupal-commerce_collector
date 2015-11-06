<?php

/**
 * @file
 * Move this functions away from module file to save some resources.
 */

/**
 * Activate commerce_collector transactions.
 *
 * @param object $order
 *   Order entity.
 *
 * @return bool
 *   TRUE or FALSE.
 */
function _commerce_collector_commerce_order_activate($order) {
  // Find all invoices which can be activated.
  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'commerce_payment_transaction')
    ->propertyCondition('order_id', $order->order_id)
    ->propertyCondition('payment_method', 'commerce_collector')
    ->propertyCondition('remote_status', COMMERCE_COLLECTOR_PAYMENT_STATUS_PENDING);
  $result = $query->execute();

  // Inform only users with update perm to payment transaction.
  $inform = commerce_payment_transaction_order_access('update', $order);

  if (isset($result['commerce_payment_transaction']) && count($result['commerce_payment_transaction']) == 1) {

    // Get transaction id.
    $transaction_ids = array_keys($result['commerce_payment_transaction']);

    // Load transaction entity.
    $transaction = commerce_payment_transaction_load(reset($transaction_ids));

    // Process, activate transaction.
    return commerce_collector_activate_invoice($transaction);
  }
  elseif (isset($result['commerce_payment_transaction']) && count($result['commerce_payment_transaction']) > 1) {
    if ($inform) {
      drupal_set_message(t('Unable to auto-activate more than one invoice at the same time. <a href="@href">Manage it manually.</a>', array('@href' => url('admin/commerce/orders/' . $order->order_id . '/payment'))), 'warning');
    }
  }
  return FALSE;
}

/**
 * Deactivate commerce_collector transactions.
 *
 * @param object $order
 *   Order entity.
 *
 * @return bool
 *   TRUE or FALSE.
 */
function _commerce_collector_commerce_order_deactivate($order) {
  // Find all invoices which can be activated.
  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'commerce_payment_transaction')
    ->propertyCondition('order_id', $order->order_id)
    ->propertyCondition('payment_method', 'commerce_collector')
    ->propertyCondition('remote_status', COMMERCE_COLLECTOR_PAYMENT_STATUS_REJECTED, '!=');
  $result = $query->execute();

  // Inform only users with update perm to payment transaction.
  $inform = commerce_payment_transaction_order_access('update', $order);

  if (isset($result['commerce_payment_transaction']) && count($result['commerce_payment_transaction']) == 1) {

    // Get transaction id.
    $transaction_ids = array_keys($result['commerce_payment_transaction']);

    // Load transaction entity.
    $transaction = commerce_payment_transaction_load(reset($transaction_ids));

    // Process, activate transaction.
    return commerce_collector_deactivate_invoice($transaction);
  }
  elseif (isset($result['commerce_payment_transaction']) && count($result['commerce_payment_transaction']) > 1) {
    if ($inform) {
      drupal_set_message(t('Unable to auto-deactivate more than one invoice at the same time. <a href="@href">Manage it manually.</a>', array('@href' => url('admin/commerce/orders/' . $order->order_id . '/payment'))), 'warning');
    }
  }
  return FALSE;
}

/**
 * Activate invoice for payment.
 */
function commerce_collector_activate_invoice($transaction) {

  // Load payment method instance to retrieve instance settings.
  $payment_method = commerce_payment_method_instance_load($transaction->instance_id);

  // If invoice was already canceled.
  if ($transaction->remote_status == COMMERCE_COLLECTOR_PAYMENT_STATUS_ACTIVATED && $transaction->status == COMMERCE_PAYMENT_STATUS_SUCCESS) {
    return TRUE;
  }

  // Remote status was changed but local was outdated.
  if ($transaction->remote_status == COMMERCE_COLLECTOR_PAYMENT_STATUS_ACTIVATED) {
    commerce_collector_save_transaction($transaction);
    return TRUE;
  }
  try {
    $data = array(
      'CountryCode' => $transaction->data['pane_values']['country'],
      'InvoiceNo' => $transaction->remote_id,
    );
    $result = _commerce_collector_request($data, $payment_method, 'ActivateInvoice');
    $transaction->remote_status = COMMERCE_COLLECTOR_PAYMENT_STATUS_ACTIVATED;
    $transaction->data['invoice'] = (array) $result;
    commerce_collector_save_transaction($transaction);
    drupal_set_message(t('Invoice @number has been activated.', array('@number' => $transaction->remote_id)));
    return TRUE;
  }
  catch (Exception $e) {
    $uri = commerce_payment_ui_payment_transaction_uri($transaction);
    if (isset($e->faultcode) && $e->faultcode == "s:INVOICE_NOT_FOUND") {
      drupal_set_message(t('Unable to change status of <a href=@href>payment transaction</a>. You cannot change status of invoice which has been already rejected. Create new invoice instead.', array('@href' => url($uri['path']))), 'error');
    }
    else {
      drupal_set_message(t('Unable to change status of <a href=@href>payment transaction</a>. Please contact administrator.', array('@href' => url($uri['path']))), 'error');
    }
    watchdog('commerce_collector', print_r($e, TRUE), array(), WATCHDOG_ERROR);
    return FALSE;
  }
}

/**
 * Deactivate invoice for payment.
 */
function commerce_collector_deactivate_invoice($transaction) {

  // Load payment method instance to retrieve instance settings.
  $payment_method = commerce_payment_method_instance_load($transaction->instance_id);

  // If invoice was already canceled.
  if ($transaction->remote_status == COMMERCE_COLLECTOR_PAYMENT_STATUS_REJECTED && $transaction->status == COMMERCE_PAYMENT_STATUS_FAILURE) {
    return TRUE;
  }

  // Remote status was changed but local was outdated.
  if ($transaction->remote_status == COMMERCE_COLLECTOR_PAYMENT_STATUS_REJECTED) {
    commerce_collector_save_transaction($transaction);
    return TRUE;
  }
  try {
    $data = array(
      'CountryCode' => $transaction->data['pane_values']['country'],
      'InvoiceNo' => $transaction->remote_id,
    );
    // Try to deactivate invoice.
    $result = _commerce_collector_request($data, $payment_method, 'CancelInvoice');
    // If succeed - change transaction status.
    $transaction->remote_status = COMMERCE_COLLECTOR_PAYMENT_STATUS_REJECTED;
    $transaction->data['invoice'] = (array) $result;
    commerce_collector_save_transaction($transaction);
    drupal_set_message(t('Invoice @number has been rejected.', array('@number' => $transaction->remote_id)));
    return TRUE;
  }
  catch (Exception $e) {
    $uri = commerce_payment_ui_payment_transaction_uri($transaction);
    if (isset($e->faultcode) && $e->faultcode == "s:INVALID_INVOICE_STATUS") {
      // In case Invoice was activated before try credit it.
      return commerce_collector_credit_invoice($transaction);
    }
    else {
      drupal_set_message(t('Unable to change status of <a href=@href>payment transaction</a>. Please contact administrator.', array('@href' => url($uri['path']))), 'error');
      watchdog('commerce_collector', print_r($e, TRUE), array(), WATCHDOG_ERROR);
      return FALSE;
    }
  }
}

/**
 * Credit an invoice.
 */
function commerce_collector_credit_invoice($transaction) {

  $payment_method = commerce_payment_method_instance_load($transaction->instance_id);

  try {
    $data = array(
      'CountryCode' => $transaction->data['pane_values']['country'],
      'InvoiceNo' => $transaction->remote_id,
      'CreditDate' => format_date(REQUEST_TIME, 'custom', 'Y-m-d\TH:i:sP'),
    );
    $result = _commerce_collector_request($data, $payment_method, 'CreditInvoice');
    $transaction->remote_status = COMMERCE_COLLECTOR_PAYMENT_STATUS_REJECTED;
    $transaction->data['invoice'] = (array) $result;
    commerce_collector_save_transaction($transaction);
    drupal_set_message(t('Invoice @number has been credited.', array('@number' => $transaction->remote_id)));
    return TRUE;
  }
  catch (Exception $e) {
    $uri = commerce_payment_ui_payment_transaction_uri($transaction);
    drupal_set_message(t('Unable to change status of <a href=@href>payment transaction</a>. Please contact administrator.', array('@href' => url($uri['path']))), 'error');
    watchdog('commerce_collector', print_r($e, TRUE), array(), WATCHDOG_ERROR);
    return FALSE;
  }
}

/**
 * Send request to Collector endpoint and get response.
 *
 * If you want to trigger this function wrap it with try..catch because it will
 * return Exception in case of any problems with request.
 *
 * @param array $data
 *   Array of arguments passed to soap call.
 * @param string $method
 *   Method name should be consistent with Collector specs.
 * @param string $wsdl
 *   WSDL filename corresponding to $method you use.
 * @param string $schema
 *   Name of SOAP schema corresponding to wsdl file.
 *
 * @return mixed
 *   Exception or result object.
 */
function _commerce_collector_request(array $data, $payment_method, $method, $wsdl = 'InvoiceServiceV32.svc', $schema = 'InvoiceService') {

  _commerce_collector_defaults($payment_method['settings']);

  $environment = $payment_method['settings']['commerce_collector_login']['commerce_collector_environment'];

  $client = new SoapClient($environment . $wsdl . '?singleWsdl', array(
    'trace' => TRUE,
    'features' => SOAP_SINGLE_ELEMENT_ARRAYS,
  ));

  $headers = array();
  $headers[] = new SoapHeader('http://schemas.ecommerce.collector.se/v30/' . $schema, 'ClientIpAddress', $_SERVER['SERVER_ADDR']);
  $headers[] = new SoapHeader('http://schemas.ecommerce.collector.se/v30/' . $schema, 'Username', $payment_method['settings']['commerce_collector_login']['commerce_collector_username']);
  $headers[] = new SoapHeader('http://schemas.ecommerce.collector.se/v30/' . $schema, 'Password', $payment_method['settings']['commerce_collector_login']['commerce_collector_password']);
  $client->__setSoapHeaders($headers);

  $result = $client->{$method}($data);

  return $result;
}

/**
 * Adds or removes fee line item from order.
 *
 * @param $payment_method
 *   Payment method instance object.
 * @param $order
 *   Order object.
 * @param bool $delete
 *   Boolean indicator if fee line item should be a part of order or not.
 *
 * @return bool
 *   Boolean status - TRUE if fee exists, FALSE if not.
 */
function commerce_collector_fee_line_item($payment_method, $order, $delete = FALSE) {

  if (!commerce_collector_line_item_exists($order) && !$delete) {

    if ($payment_method['settings']['commerce_collector_login']['commerce_collector_fee']) {

      // Create the new line item.
      $line_item = commerce_line_item_new('collector_fee', $order->order_id);

      // Wrap the line item to simplify manipulating their field data.
      $line_item_wrapper = entity_metadata_wrapper('commerce_line_item', $line_item);

      // Populate the $line_item_wrapper...
      $line_item_wrapper->line_item_label = t('Invoice fee');
      $line_item_wrapper->quantity = 1;
      $line_item_wrapper->commerce_unit_price->amount = $payment_method['settings']['commerce_collector_login']['commerce_collector_fee_amount'];
      $line_item_wrapper->commerce_unit_price->currency_code = commerce_default_currency();

      // Set the price component of the unit price.
      $line_item_wrapper->commerce_unit_price->data = commerce_price_component_add(
        $line_item_wrapper->commerce_unit_price->value(),
        'collector_fee',
        $line_item_wrapper->commerce_unit_price->value(),
        TRUE,
        FALSE
      );
      commerce_line_item_save($line_item);

      $order_wrapper = entity_metadata_wrapper('commerce_order', $order);
      $order_wrapper->commerce_line_items[] = $line_item;
      return commerce_order_save($order);
    }
  }
  elseif (commerce_collector_line_item_exists($order) && $delete) {
    // @todo Remove line item here.
  }
  return FALSE;
}

/**
 * Creates an example payment transaction for the specified charge amount.
 *
 * @param object $transaction
 *   New or existing transaction.
 *
 * @return bool
 *   TRUE if saved, FALSE if not.
 *
 * @deprecated
 *   Use commerce_collector_set_status than commerce_payment_transaction_save.
 */
function commerce_collector_save_transaction(&$transaction) {
  commerce_collector_set_status($transaction);
  return commerce_payment_transaction_save($transaction);
}

/**
 * Change local status in reference to remote status.
 *
 * @param $transaction
 *   (Reference) Transaction object.
 */
function commerce_collector_set_status(&$transaction) {
  switch ($transaction->remote_status) {
    case COMMERCE_COLLECTOR_PAYMENT_STATUS_ACTIVATED:
      $transaction->status = COMMERCE_PAYMENT_STATUS_SUCCESS;
      $transaction->message = 'Invoice @number has been activated.';
      break;

    case COMMERCE_COLLECTOR_PAYMENT_STATUS_ON_HOLD:
    case COMMERCE_COLLECTOR_PAYMENT_STATUS_PENDING:
    case COMMERCE_COLLECTOR_PAYMENT_STATUS_SIGNING:
      $transaction->status = COMMERCE_PAYMENT_STATUS_PENDING;
      $transaction->message = 'Invoice @number has a pending status (@status) now';
      break;

    case COMMERCE_COLLECTOR_PAYMENT_STATUS_REJECTED:
      $transaction->status = COMMERCE_PAYMENT_STATUS_FAILURE;
      $transaction->message = 'Invoice @number has been rejected.';
      break;
  }
  $transaction->message_variables = array(
    '@number' => $transaction->remote_id,
    '@status' => $transaction->remote_status,
  );
}

/**
 * Calculate VAT for line item.
 *
 * Two methods available - VAT or Fixed Vat Rate (configurable with variable).
 */
function _commerce_collector_line_item_vat($payment_method, $unit_price) {
  $use_vat_module = $payment_method['settings']['commerce_collector_vat']['commerce_collector_use_vat_module'];
  // Use vat module based rate or fixed one.
  if (!$use_vat_module) {
    return $payment_method['settings']['commerce_collector_vat']['commerce_collector_fixed_vat_rate'];
  }
  // Check if price has components.
  if (!isset($unit_price['data']['components'])) {
    return 0;
  }
  // Get all VAT components.
  $components = $unit_price['data']['components'];
  $components = commerce_vat_components($components);
  if (empty($components)) {
    return 0;
  }
  // Get first element only. More than one VAT rate per item is not a common
  // case.
  $vat = reset($components);
  if (!isset($vat['price']['data']['vat_rate_info']['rate'])) {
    return 0;
  }
  // Return vat rate (in percents).
  return $vat['price']['data']['vat_rate_info']['rate'] * 100;
}
